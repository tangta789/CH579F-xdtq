#ifndef __GPIO_TO_I2C_H__
#define __GPIO_TO_I2C_H__

// NOTE: if you embedded porting ,please modify this header file
#include "CH57x_common.h"
#define GPIO_TO_I2C_DEBUG 0
#if GPIO_TO_I2C_DEBUG
#define GPIO_TO_I2C_PRINT  PRINT
#else
#define GPIO_TO_I2C_PRINT(fmt,...)  
#endif

#ifdef __cplusplus
 extern "C" {
#endif

/****************************************************
* return UINT32 define
* -------------
* return format(31-0): status|err number|SAD + W/R|RA|INDEX
*   status: 1bit, 0 for OK, 1 for ERR
*   err number: 3bits, 0x1 for not recv SAD + W ACK
*   SAD + W/R: 8bits, Slave ID and Read/Write flag
*   RA: 8bits, regist address
*   INDEX: 12bits, err index for send/recv
*   
*****************************************************/
//"init failed", //  ERR_GPIO_TO_I2C_INIT                                    0xF0000000     01111
//"cannot recv ack of write SAD + W", // ERR_GPIO_TO_I2C_NO_W_ACK            0xC0000000     01100
//"cannot recv ack of read SAD + W",// ERR_GPIO_TO_I2C_NO_R_ACK              0x80000000     01000
//"cannot recv ack of write regiter address",// ERR_GPIO_TO_I2C_NO_W_RA_ACK  0xE0000000     01110
//"cannot recv ack of read regiter address",// ERR_GPIO_TO_I2C_NO_R_RA_ACK   0xA0000000     01010
//"cannot recv ack of read SAD + R",// ERR_GPIO_TO_I2C_NO_R_ACK2             0xD0000000     01101
//"cannot recv ack of write data",// ERR_GPIO_TO_I2C_NO_W_DATA_ACK           0xB0000000     01011
#define OK_GPIO_TO_I2C                   0x0
#define ERR_GPIO_TO_I2C_INIT             0xF0000000
#define ERR_GPIO_TO_I2C_NO_W_ACK         0x80000000
#define ERR_GPIO_TO_I2C_NO_R_ACK         0xC0000000 
#define ERR_GPIO_TO_I2C_NO_W_RA_ACK      0xE0000000
#define ERR_GPIO_TO_I2C_NO_R_RA_ACK      0xA0000000 
#define ERR_GPIO_TO_I2C_NO_W_DATA_ACK    0xD0000000
#define ERR_GPIO_TO_I2C_NO_R_ACK2        0xB0000000 

#define GPIO_TO_I2C_GetERR(ret)     (ret & 0xF0000000)
#define GPIO_TO_I2C_GetSAD(ret)     ((ret>>20) & 0xFF)
#define GPIO_TO_I2C_GetRA(ret)     ((ret>>12) & 0xFF)
#define GPIO_TO_I2C_GetIndex(ret)     ((ret) & 0xFFF)
#define GPIO_TO_I2C_HasERR(ret) (GPIO_TO_I2C_GetERR(ret) != OK_GPIO_TO_I2C)
#define GPIO_TO_I2C_SetERR(err,SAD,RA,index)   ( err | (((SAD)<<20) & 0xFF00000) | (((RA)<<12) & 0xFF000)| ((index) & 0x0FFF) )


struct gpio_to_i2c_t {
	void (*set_input)(UINT32 pin);
	void (*set_output)(UINT32 pin);
	void (*output_high)(UINT32 pin);
	void (*output_low)(UINT32 pin);
	BOOL (*input_read)(UINT32 pin);
	void (*delay_us)(UINT16 us);
	UINT32 timeout;   // default is 5 us
	UINT32 SDA;
	UINT32 SCL;
};

UINT32 GPIO_TO_I2C_Init(struct gpio_to_i2c_t *handle);
/***********************************************************
* The Master is writing one byte to the Slave
* ------------------------------------------------------
* |Master| S | SAD + W |   |  RA  |   |  DATA  |   | P |
* ------------------------------------------------------
* |Slave |   |         |ACK|      |ACK|        |ACK|   |
* ------------------------------------------------------
***********************************************************/
#define GPIO_TO_I2C_SendOneByte(handle,SAD,RA,data)  GPIO_TO_I2C_SendMulBytes(handle,SAD,RA,data,1)
/********************************************************************
* The Master is writing multiple bytes to the Slave
* -------------------------------------------------------------------
* |Master| S | SAD + W |   |  RA  |   |  DATA  |   |  DATA  |   | P |
* -------------------------------------------------------------------
* |Slave |   |         |ACK|      |ACK|        |ACK|        |ACK|   |
* -------------------------------------------------------------------
*********************************************************************/
UINT32 GPIO_TO_I2C_SendMulBytes(struct gpio_to_i2c_t *handle,UINT8 SAD,UINT8 RA,UINT8* data,UINT32 size);
/***************************************************************************
* The Master is receiving one byte of data from the Slave
* --------------------------------------------------------------------------
* |Master| S | SAD + W |   |  RA  |   | Sr | SAD + R |   |        |NACK| P |
* --------------------------------------------------------------------------
* |Slave |   |         |ACK|      |ACK|    |         |ACK|  DATA  |    |   |
* --------------------------------------------------------------------------
****************************************************************************/
#define GPIO_TO_I2C_RecvOneByte(handle,SAD,RA,data)  GPIO_TO_I2C_RecvMulBytes(handle,SAD,RA,data,1)
/****************************************************************************************
* The Master is receiving multiple bytes of data from the Slave
* ---------------------------------------------------------------------------------------
* |Master| S | SAD + W |   |  RA  |   | Sr | SAD + R |   |        |ACK|        |NACK| P |
* ---------------------------------------------------------------------------------------
* |Slave |   |         |ACK|      |ACK|    |         |ACK|  DATA  |   |  DATA  |    |   |
* ---------------------------------------------------------------------------------------
*****************************************************************************************/
UINT32 GPIO_TO_I2C_RecvMulBytes(struct gpio_to_i2c_t *handle,UINT8 SAD,UINT8 RA,UINT8 *data, UINT32 size);


#ifdef __cplusplus
 }
#endif
#endif
