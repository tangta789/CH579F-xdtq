/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : Bigan(tangan@magicore.tech)
* Version            : V1.0
* Date               : 2021/04/21
* Description 		   : 
*******************************************************************************/

#include "CH57x_common.h"
#include "kxtj9.h"


#if 0
#define DEBUG_PRINT(fmt,...) 
#else
#define DEBUG_PRINT PRINT
#endif

void DebugInit(void)		
{
 
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}
/* */
volatile UINT32 led1Flag = 0;
volatile UINT32 led1Curr = 0;
volatile UINT32 led2Flag = 0;
volatile UINT32 led2Curr = 0;
#define LED1  GPIO_Pin_4
#define LED2  GPIO_Pin_12
/**
* ---------------------------------
* 31              15              0
* +-------+-------+-------+-------+
* | times |lightup     | lightdown|
* +-------+-------+-------+-------+
* | 8 bits|  12bits    |  12 bits |
* ---------------------------------
* Flag is UINT32, High 8 bits is times, middle 12 bits is lightup timeout, low 12 bits is lightdown timeout
* when flag is set, we check whether last bling is running. if not, goto init time0, else just set flag.
* In IRQ handler function, we check whether has new data at first. if yes, drop last task, set new data to work. when task is over, we close time0.
* time0 just for bling LEDs and buzzer,timeout is 5ms, so we just support lightup_timeout (4096 - 2) * 5ms = 20470ms 
* 0xFFF of lightup_timeout is light up always
*/
#define TIMR0_TIMEOUT    5
void CheckStartTime0()
{
	if (led1Curr == 0 && led2Curr == 0) {
		PRINT("start time0\r\n");
		led1Curr = led1Flag;
		led2Curr = led2Flag;
			// time0 timeout is 5ms
		TMR0_TimerInit(FREQ_SYS/(1000 / TIMR0_TIMEOUT));      
		TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    NVIC_EnableIRQ(TMR0_IRQn);
	}
}
void CheckStopTime0()
{
	if (led1Curr == 0 && led2Curr == 0) {
		PRINT("stop time0\r\n");
		led1Flag = led2Flag;
		// time0 timeout is 5ms      
    TMR0_Disable();
    NVIC_DisableIRQ(TMR0_IRQn);
	}
}
void LedLightBling(UINT32 led,UINT16 lightup_timeout,UINT16 lightdown_timeout,UINT8 times)
{
	if (times != 0) {
		if (led == LED1) {
			led1Flag = (0xFF000000 & (times << 24)) | (0xFFF000 & ((lightup_timeout / TIMR0_TIMEOUT) << 12)) |  (0xFFF & (lightdown_timeout / TIMR0_TIMEOUT));
		} else if (led == LED2) {
			led2Flag = (0xFF000000 & (times << 24)) | (0xFFF000 & ((lightup_timeout / TIMR0_TIMEOUT) << 12)) |  (0xFFF & (lightdown_timeout / TIMR0_TIMEOUT));
		}
		CheckStartTime0();
	}
}
#if 1
#define GET_TIMES(flag)   rep_times=((flag>>24)&0xFF);work_time=((flag>>12)&0xFFF);idl_time=((flag)&0xFFF)
#else
#define GET_TIMES(flag)   rep_times=((flag>>24)&0xFF);work_time=((flag>>12)&0xFFF);idl_time=((flag)&0xFFF);PRINT("times=%d,work_time=%d,idl_time=%d,flag=0x%lx\r\n",rep_times,work_time,idl_time,flag)

#endif
#if 0
void TMR0_IRQHandler( void )        // TMR0 定时中断
{
    if( TMR0_GetITFlag( TMR0_3_IT_CYC_END ) )
    {
        TMR0_ClearITFlag( TMR0_3_IT_CYC_END );      // 清除中断标志
        GPIOB_InverseBits( GPIO_Pin_4 );   			
			PRINT( "bling led\r\n" );
    }
}
#else
void TMR0_IRQHandler(void)
{
	UINT8 rep_times;
	UINT16 work_time;
	UINT16 idl_time;
	if( TMR0_GetITFlag( TMR0_3_IT_CYC_END ) ) {
		TMR0_ClearITFlag( TMR0_3_IT_CYC_END );      // 清除中断标志
		#if 1
		// whether has new data
		if ((led1Flag & 0xFF000000) != 0) {
			DEBUG_PRINT("find led1 new data 0x%08lx,old data 0x%08lx\r\n",led1Flag,led1Curr);
			led1Curr = led1Flag;
			led1Flag &= 0xFFFFFF;
		}
		if ((led2Flag & 0xFF000000) != 0) {
			DEBUG_PRINT("find led2 new data 0x%08lx,old data 0x%08lx\r\n",led2Flag,led2Curr);
			led2Curr = led2Flag;
			led2Flag &= 0xFFFFFF;
		}
		// check LED1
		GET_TIMES(led1Curr);
		if (rep_times > 0x00) {
			// lightup LED1
			if (work_time > 0) {
				if (work_time != 0xFFF) work_time--;
				if (work_time == 0) {
					GPIOB_SetBits(LED1);
					DEBUG_PRINT("change LED1 to lightdown\r\n");
				} else {
					GPIOB_InverseBits(LED1);
				}
				led1Curr = (0xFF000000 & (rep_times << 24)) | ((work_time << 12) & 0xFFF000) | (0xFFF & led1Flag);
			// lightdown LED1
			} else if (idl_time > 0) {
				if (idl_time != 0xFFF) idl_time--;
				if (idl_time == 0) {
					if (rep_times == 0xFF || --rep_times > 0) {
						GPIOB_InverseBits(LED1);
						led1Curr = (0xFF000000 & (rep_times << 24)) | (0xFFFFFF & led1Flag);
						DEBUG_PRINT("change LED1 to lightup\r\n");
					} else {
						GPIOB_SetBits(LED1);
						led1Curr = 0;
					}
				} else {
					led1Curr = (0xFF000000 & (rep_times << 24)) | (0xFFF & idl_time);
				}
			} else {
				// lightdown LED1
				GPIOB_SetBits(LED1);
				led1Curr = 0;
				PRINT("error LED1\r\n");
			}
		}
		// check LED2
		GET_TIMES(led2Curr);
		if (rep_times > 0x00) {
			// lightup LED1
			if (work_time > 0) {
				if (work_time != 0xFFF) work_time--;
				if (work_time == 0) {
					GPIOA_SetBits(LED2);
					DEBUG_PRINT("change LED2 to lightdown\r\n");
				} else {
					GPIOA_InverseBits(LED2);
				}
				led2Curr = (0xFF000000 & (rep_times << 24)) | ((work_time << 12) & 0xFFF000) | (0xFFF & led2Flag);
			// lightdown LED2
			} else if (idl_time > 0) {
				if (idl_time != 0xFFF) idl_time--;
				if (idl_time == 0) {
					if (rep_times == 0xFF || --rep_times > 0) {
						GPIOA_InverseBits(LED2);
						led2Curr = (0xFF000000 & (rep_times << 24)) | (0xFFFFFF & led2Flag);
						DEBUG_PRINT("change LED2 to lightup\r\n");
					} else {
						GPIOA_SetBits(LED2);
						led2Curr = 0;
					}
				} else {
					led2Curr = (0xFF000000 & (rep_times << 24)) | (0xFFF & idl_time);
				}
			} else {
				// lightdown LED2
				GPIOA_SetBits(LED2);
				led2Curr = 0;
				PRINT("error LED2\r\n");
			}
		}
		//PRINT("led1Curr=0x%08lx,led2Curr=0x%08lx,buzzerCurr=0x%08lx\r\n",led1Curr,led2Curr,buzzerCurr);
		CheckStopTime0();
		#endif
	}
}
#endif
void BuzzerRing(UINT32 time)
{
//#define BUZZER_HZ  2000
#define BUZZER_HZ  4000

	UINT32 high_times = 0;
	UINT32 i;
	UINT32 times = BUZZER_HZ / 500;
	UINT32 delay = 500000 /  BUZZER_HZ;
	// 2000Hz   500us one cycle
	for(;time > 0;(time == 0xFFFFFFFF ? time: time--)) {
		for(i = 0;i < times;i++) {
			if (high_times++ % 10 == 0) {
				high_times = 0;
				GPIOA_InverseBits(GPIO_Pin_14);
			} 
			DelayUs(delay);
		}
	}
	GPIOA_ResetBits(GPIO_Pin_14);
}
int main()
{
	UINT16 x,y,z;
//  UINT32 ret;
  /* Init Uart1 for log output */   
	DebugInit();
	PRINT( "\r\nXiaoDing Start @ChipID=%02X version: v0.18\r\n", R8_CHIP_ID );
	
	#if 1
	/************************************************************************/
	DEBUG_PRINT("Init devices:\r\n");
	/* init devices: leds, buzzer and sensor */
	// LED1 Yellow PB4/RXD0/PWM7
	GPIOB_ModeCfg(LED1, GPIO_ModeOut_PP_5mA);
	GPIOB_SetBits(LED1);
	DEBUG_PRINT("LED1 PB4:0x%08x\r\n",LED1);
	// LED2 Red PA12/SCS/PWM4/AIN2
	GPIOA_ModeCfg(LED2, GPIO_ModeOut_PP_5mA);
	GPIOA_SetBits(LED2);
	DEBUG_PRINT("LED2 PB12:0x%08x\r\n",LED2);

	/* init led and buzzer: buzzer */
	// Buzzer PA14/MOSI/TXD0_/AIN4
	GPIOA_ModeCfg(GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
	GPIOA_SetBits(GPIO_Pin_14);
	DEBUG_PRINT("Buzzer PA14:0x%08x\r\n",GPIO_Pin_14);

	/************************************************************************/
	DEBUG_PRINT("Send start message by led\r\n");
	LedLightBling(LED1,1000,TIMR0_TIMEOUT,1);
	LedLightBling(LED2,1000,TIMR0_TIMEOUT,1);
	
	if (!KXTJ9_Restart()) {
			PRINT("KXTJ9 enable error\r\n");
			// BuzzerRing(0xFFFFFFFF);
			BuzzerRing(3000);
	} else {
			DEBUG_PRINT("KXTJ9 enable success\r\n");
			BuzzerRing(1000);
	}
	PRINT("Start main task\r\n");
	while(1) {
		if (KXTJ9_HasData()) {
			if (KXTJ9_ReadData(&z,&y,&z)) {
				PRINT("(x,y,z) (%d,%d,%d)\r\n",x,y,z);
			}
		}
		DelayMs(100);
	}
	#else
	  GPIOB_SetBits( GPIO_Pin_4 );
    GPIOB_ModeCfg( GPIO_Pin_4, GPIO_ModeOut_PP_5mA );
    
    TMR0_TimerInit( FREQ_SYS/10 );                  // 设置定时时间 100ms
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);          // 开启中断
    NVIC_EnableIRQ( TMR0_IRQn );
		while(1);
    
	#endif
}



