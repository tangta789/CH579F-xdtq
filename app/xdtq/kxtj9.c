#include "CH57x_common.h"
#include "kxtj9.h"
#include "gpio_to_i2c.h"

// The KXTJ9's Slave Address is 0001111
#define KIONIX_ACCEL_I2C_ADDR		0x0F
#define KIONIX_ACCEL_NAME			"kionix_accel"
#define KIONIX_ACCEL_IRQ			"kionix-irq
/******************************************************************************
 * Accelerometer WHO_AM_I return value
 *****************************************************************************/
#define KIONIX_ACCEL_WHO_AM_I_KXTE9         0x00
#define KIONIX_ACCEL_WHO_AM_I_KXTF9         0x01
#define KIONIX_ACCEL_WHO_AM_I_KXTI9_1001    0x04
#define KIONIX_ACCEL_WHO_AM_I_KXTIK_1004    0x05
#define KIONIX_ACCEL_WHO_AM_I_KXTJ9_1005    0x07
#define KIONIX_ACCEL_WHO_AM_I_KXTJ9_1007    0x08
#define KIONIX_ACCEL_WHO_AM_I_KXCJ9_1008    0x0A
#define KIONIX_ACCEL_WHO_AM_I_KXTJ2_1009    0x09
#define KIONIX_ACCEL_WHO_AM_I_KXCJK_1013    0x11
/******************************************************************************
 * Accelerometer Grouping
 *****************************************************************************/
#define KIONIX_ACCEL_GRP1   1   /* KXTE9 */
#define KIONIX_ACCEL_GRP2   2   /* KXTF9/I9-1001/J9-1005 */
#define KIONIX_ACCEL_GRP3   3   /* KXTIK-1004 */
#define KIONIX_ACCEL_GRP4   4   /* KXTJ9-1007/KXCJ9-1008 */
#define KIONIX_ACCEL_GRP5   5   /* KXTJ2-1009 */
#define KIONIX_ACCEL_GRP6   6   /* KXCJK-1013 */

/******************************************************************************
 * Registers for Accelerometer Group 1 & 2 & 3
 *****************************************************************************/
#define ACCEL_WHO_AM_I      0x0F
#define ACCEL_DCST_RESP     0x0C


/*****************************************************************************/
/* Registers for Accelerometer Group 1 */
/*****************************************************************************/
/* Output Registers */
#define ACCEL_GRP1_XOUT         0x12
/* Control Registers */
#define ACCEL_GRP1_CTRL_REG1    0x1B
/* CTRL_REG1 */
#define ACCEL_GRP1_PC1_OFF      0x7F
#define ACCEL_GRP1_PC1_ON       (1 << 7)
#define ACCEL_GRP1_ODR40        (3 << 3)
#define ACCEL_GRP1_ODR10        (2 << 3)
#define ACCEL_GRP1_ODR3         (1 << 3)
#define ACCEL_GRP1_ODR1         (0 << 3)
#define ACCEL_GRP1_ODR_MASK     (3 << 3)

/*****************************************************************************/
/* Registers for Accelerometer Group 2 & 3 */
/*****************************************************************************/
/* Output Registers */
#define ACCEL_GRP2_XOUT_L       0x06
/* Control Registers */
#define ACCEL_GRP2_INT_REL      0x1A
#define ACCEL_GRP2_CTRL_REG1    0x1B
#define ACCEL_GRP2_INT_CTRL1    0x1E
#define ACCEL_GRP2_DATA_CTRL    0x21
/* CTRL_REG1 */
#define ACCEL_GRP2_PC1_OFF      0x7F
#define ACCEL_GRP2_PC1_ON       (1 << 7)
#define ACCEL_GRP2_DRDYE        (1 << 5)
#define ACCEL_GRP2_G_8G         (2 << 3)
#define ACCEL_GRP2_G_4G         (1 << 3)
#define ACCEL_GRP2_G_2G         (0 << 3)
#define ACCEL_GRP2_G_MASK       (3 << 3)
#define ACCEL_GRP2_RES_8BIT     (0 << 6)
#define ACCEL_GRP2_RES_12BIT    (1 << 6)
#define ACCEL_GRP2_RES_MASK     (1 << 6)
/* INT_CTRL1 */
#define ACCEL_GRP2_IEA          (1 << 4)
#define ACCEL_GRP2_IEN          (1 << 5)
/* DATA_CTRL_REG */
#define ACCEL_GRP2_ODR12_5      0x00
#define ACCEL_GRP2_ODR25        0x01
#define ACCEL_GRP2_ODR50        0x02
#define ACCEL_GRP2_ODR100       0x03
#define ACCEL_GRP2_ODR200       0x04
#define ACCEL_GRP2_ODR400       0x05
#define ACCEL_GRP2_ODR800       0x06
/*****************************************************************************/

/*****************************************************************************/
/* Registers for Accelerometer Group 4 & 5 & 6 */
/*****************************************************************************/
/* Output Registers */
#define ACCEL_GRP4_XOUT_L       0x06
/* Control Registers */
#define ACCEL_GRP4_INT_REL      0x1A
#define ACCEL_GRP4_CTRL_REG1    0x1B
#define ACCEL_GRP4_INT_CTRL1    0x1E
#define ACCEL_GRP4_DATA_CTRL    0x21
/* CTRL_REG1 */
#define ACCEL_GRP4_PC1_OFF      0x7F
#define ACCEL_GRP4_PC1_ON       (1 << 7)
#define ACCEL_GRP4_DRDYE        (1 << 5)
#define ACCEL_GRP4_G_8G         (2 << 3)
#define ACCEL_GRP4_G_4G         (1 << 3)
#define ACCEL_GRP4_G_2G         (0 << 3)
#define ACCEL_GRP4_G_MASK       (3 << 3)
#define ACCEL_GRP4_RES_8BIT     (0 << 6)
#define ACCEL_GRP4_RES_12BIT    (1 << 6)
#define ACCEL_GRP4_RES_MASK     (1 << 6)
/* INT_CTRL1 */
#define ACCEL_GRP4_IEA          (1 << 4)
#define ACCEL_GRP4_IEN          (1 << 5)
/* DATA_CTRL_REG */
#define ACCEL_GRP4_ODR0_781     0x08
#define ACCEL_GRP4_ODR1_563     0x09
#define ACCEL_GRP4_ODR3_125     0x0A
#define ACCEL_GRP4_ODR6_25      0x0B
#define ACCEL_GRP4_ODR12_5      0x00
#define ACCEL_GRP4_ODR25        0x01
#define ACCEL_GRP4_ODR50        0x02
#define ACCEL_GRP4_ODR100       0x03
#define ACCEL_GRP4_ODR200       0x04
#define ACCEL_GRP4_ODR400       0x05
#define ACCEL_GRP4_ODR800       0x06
#define ACCEL_GRP4_ODR1600      0x07
/*****************************************************************************/

/* Input Event Constants */
#define ACCEL_G_MAX         8096
#define ACCEL_FUZZ          3
#define ACCEL_FLAT          3
/* I2C Retry Constants */
#define KIONIX_I2C_RETRY_COUNT      10  /* Number of times to retry i2c */
#define KIONIX_I2C_RETRY_TIMEOUT    1   /* Timeout between retry (miliseconds) */

/* Earlysuspend Contants */
#define KIONIX_ACCEL_EARLYSUSPEND_TIMEOUT   5000    /* Timeout (miliseconds) */
/*
 * The following table lists the maximum appropriate poll interval for each
 * available output data rate (ODR).
 */
// this code area is I2C interface impl
#if 1
struct gpio_to_i2c_t i2c_handle;
void SetInput(UINT32 pin)
{
	GPIOB_ModeCfg(pin,GPIO_ModeIN_PD);
}
void SetOutput(UINT32 pin)
{
	GPIOB_ModeCfg(pin,GPIO_ModeOut_PP_5mA);
}
void OutputHigh(UINT32 pin)
{
	GPIOB_SetBits(pin);
}
void OutputLow(UINT32 pin)
{
	GPIOB_ResetBits(pin);
}
BOOL InputRead(UINT32 pin)
{
	return (GPIOB_ReadPortPin(pin) != 0);
}
#endif
// this code area is KXTJ9-1005 driver impl
#if 1
#endif
UINT8 axis_map_x,axis_map_y,axis_map_z;
BOOL negate_x,negate_y,negate_z;
UINT8 shift;
int accel_cali[3];
static void update_direction(UINT32 group, UINT32 direction)
{

    axis_map_x = ((direction-1)%2);
    axis_map_y =  (direction%2);
    axis_map_z =  2;
    negate_z = ((direction-1)/4);
    switch(group) {
        case KIONIX_ACCEL_GRP3:
        case KIONIX_ACCEL_GRP6:
            negate_x = (((direction+2)/2)%2);
            negate_y = (((direction+5)/4)%2);
            break;
        case KIONIX_ACCEL_GRP5:
            axis_map_x =  (direction%2);
            axis_map_y = ((direction-1)%2);
            negate_x =  (((direction+1)/2)%2);
            negate_y =  (((direction/2)+((direction-1)/4))%2);
            break;
        default:
            negate_x =  ((direction/2)%2);
            negate_y = (((direction+1)/4)%2);
            break;
    }    
    return;
}
static void update_shift(UINT8 RA_value)
{
    switch (RA_value & (~ACCEL_GRP4_G_MASK)) {
        case ACCEL_GRP4_G_8G:
            shift = 2; 
            break;
        case ACCEL_GRP4_G_4G:
            shift = 3; 
            break;
        case ACCEL_GRP4_G_2G:
        default:
            shift = 4; 
            break;
    }    

    return;
}
/* Allow users to change the calibration value of the device */
void set_calibration(INT32 x,INT32 y,INT32 z)
{
	accel_cali[axis_map_x] = x;
	accel_cali[axis_map_y] = y;
	accel_cali[axis_map_z] = z;
}
BOOL KXTJ9_Restart(void)
{
	UINT32 ret;
	UINT8 value = 0;
	// init i2c
	i2c_handle.timeout = 5;
	// PB14/TIO/MOSI_/PWM10/DSR
	i2c_handle.SDA = GPIO_Pin_14;
	// PB13/SCK0_/TXD1
	i2c_handle.SCL = GPIO_Pin_13;
	i2c_handle.delay_us = mDelayuS;
	i2c_handle.set_input = SetInput;
	i2c_handle.set_output = SetOutput;
	i2c_handle.output_high = OutputHigh;
	i2c_handle.output_low = OutputLow;
	i2c_handle.input_read = InputRead;
	GPIO_TO_I2C_Init(&i2c_handle);
	// config irq gpio input PB11/UD+/TMR2
	GPIOB_ModeCfg(GPIO_Pin_11,GPIO_ModeIN_PD);
	PRINT("GPIOB_ReadPortPin(GPIO_Pin_11)=%ld\r\n",GPIOB_ReadPortPin(GPIO_Pin_11));
	#if 1
	// check device is OK
	value = 0;
	ret = GPIO_TO_I2C_RecvOneByte(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,ACCEL_DCST_RESP,&value);
	PRINT("KXTJ9 RA 0x%02x value 0x%02x\r\n",ACCEL_DCST_RESP,value);
	if (GPIO_TO_I2C_HasERR(ret)) {
		PRINT("KXJ9 get ACCEL_DCST_RESP err 0x%08lu\r\n",ret);
		return FALSE;
	}
	if (value != 0x55) {
		PRINT("ACCEL_DCST_RESP return error[0x%02x]\r\n",value);
		return FALSE;
	}
	#endif
	// check device is right
	value = 0;
	ret = GPIO_TO_I2C_RecvOneByte(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,ACCEL_WHO_AM_I,&value);
	if (GPIO_TO_I2C_HasERR(ret)) {
		PRINT("KXJ9 get ACCEL_WHO_AM_I err 0x%08lu\r\n",ret);
		return FALSE;
	}
	if (value != KIONIX_ACCEL_WHO_AM_I_KXTJ9_1005) {
		PRINT("ACCEL_WHO_AM_I return error[0x%02x]\r\n",value);
		return FALSE;
	}
	/* ensure that PC1 is cleared before updating control registers */
	value = 0;
	ret = GPIO_TO_I2C_SendOneByte(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,ACCEL_GRP2_CTRL_REG1,&value);
	PRINT("KXTJ9 RA 0x%02x value 0x%02x\r\n",ACCEL_GRP2_CTRL_REG1,value);
	if (GPIO_TO_I2C_HasERR(ret)) {
		PRINT("KXJ9 set ACCEL_GRP2_CTRL_REG1 err 0x%08lu\r\n",ret);
		return FALSE;
	}
	// set DATA_CTRL_REG ,set output data rate, default 50Hz (ACCEL_GRP2_ODR50)
	value = ACCEL_GRP2_ODR50;
	ret = GPIO_TO_I2C_SendOneByte(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,
			ACCEL_GRP2_DATA_CTRL,&value);
		PRINT("KXTJ9 RA 0x%02x value 0x%02x\r\n",ACCEL_GRP2_DATA_CTRL,value);
	if (GPIO_TO_I2C_HasERR(ret)) {
		PRINT("KXJ9 set ACCEL_GRP2_DATA_CTRL err 0x%08lu\r\n",ret);
		return FALSE;
	}
	// INT_CTRL_REG1, set IRQ enable, default active high (ACCEL_GRP2_IEN | ACCEL_GRP2_IEA)
	value = ACCEL_GRP2_IEN | ACCEL_GRP2_IEA;
	ret = GPIO_TO_I2C_SendOneByte(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,
		ACCEL_GRP2_INT_CTRL1,&value);
	PRINT("KXTJ9 RA 0x%02x value 0x%02x\r\n",ACCEL_GRP2_INT_CTRL1,value);
	if (GPIO_TO_I2C_HasERR(ret)) {
		PRINT("KXJ9 set ACCEL_GRP2_INT_CTRL1 err 0x%08lu\r\n",ret);
		return FALSE;
	}
	// CTRL_REG1, set function gpio, default ()
	value = ACCEL_GRP2_PC1_ON | ACCEL_GRP2_RES_12BIT | ACCEL_GRP2_DRDYE | ACCEL_GRP4_G_2G;
	update_shift(value);
		/* This variable controls the corresponding direction
	 * of the accelerometer that is mounted on the board
	 * of the device. Refer to the porting guide for
	 * details. Valid value is 1 to 8. */
	update_direction(KIONIX_ACCEL_GRP2,1);
	set_calibration(0,0,0);
	ret = GPIO_TO_I2C_SendOneByte(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,
		ACCEL_GRP2_CTRL_REG1,&value);
	PRINT("KXTJ9 RA 0x%02x value 0x%02x\r\n",ACCEL_GRP2_CTRL_REG1,value);
	if (GPIO_TO_I2C_HasERR(ret)) {
		PRINT("KXJ9 set ACCEL_GRP2_CTRL_REG1 err 0x%08lu\r\n",ret);
		return FALSE;
	}
	return TRUE;
}

BOOL KXTJ9_HasData(void)
{
	UINT32 ret;
	UINT8 value = 0;
	if ((GPIOB_ReadPortPin(GPIO_Pin_11) != 0)) {
		ret = GPIO_TO_I2C_RecvMulBytes(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,
			0x18,&value,1);
		//PRINT("KXTJ9 RA 0x%02x value 0x%02x\r\n",0x18,value);
		if (GPIO_TO_I2C_HasERR(ret)) {
			PRINT("read 0x18 err 0x%08lu\r\n",ret);
			return FALSE;
		}
		//PRINT("value===%02x\n",value);
		return value != 0;
	} 
	return FALSE;
}
#define le16_to_cpu(A) (A)
BOOL KXTJ9_ReadData(UINT16 *out_x, UINT16 *out_y, UINT16 *out_z)
{
	UINT32 ret;
	UINT8  accel_data_s8[6];
  UINT16 *accel_data_s16 = (UINT16 *)accel_data_s8;
	UINT16 x,y,z;
	ret = GPIO_TO_I2C_RecvMulBytes(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,
			ACCEL_GRP2_XOUT_L,accel_data_s8,6);
	if (GPIO_TO_I2C_HasERR(ret)) {
		//PRINT("read ACCEL_GRP2_DATA_CTRL err 0x%08lu\r\n",ret);
		return FALSE;
	}
  x = ((UINT16) le16_to_cpu(accel_data_s16[axis_map_x])) >> shift;
  y = ((UINT16) le16_to_cpu(accel_data_s16[axis_map_y])) >> shift;
  z = ((UINT16) le16_to_cpu(accel_data_s16[axis_map_z])) >> shift;

  *out_x = (negate_x ? -x : x) + accel_cali[axis_map_x];
  *out_y = (negate_y ? -y : y) + accel_cali[axis_map_y];
  *out_z = (negate_z ? -z : z) + accel_cali[axis_map_z];
	ret = GPIO_TO_I2C_RecvMulBytes(&i2c_handle,KIONIX_ACCEL_I2C_ADDR,
			ACCEL_GRP4_INT_REL,accel_data_s8,1);
	//PRINT("KXTJ9 RA 0x%02x \r\n",ACCEL_GRP4_INT_REL);
	if (GPIO_TO_I2C_HasERR(ret)) {
		PRINT("read ACCEL_GRP4_INT_REL err 0x%08lu\r\n",ret);
		return FALSE;
	}
	return TRUE;
}



