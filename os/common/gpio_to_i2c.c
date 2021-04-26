#include "gpio_to_i2c.h"
#define I2C_READ     1
#define I2C_WRITE     0
#define I2C_SET_SAD_R_OR_W_FLAG(SAD,flag) ((((SAD) << 1) & 0xFE) | ((flag) & 0x1))
/**
**** start condition
*       ---------
*  SCL    
*
*       ----
*  SDA      |
*            ----
*/
static void gpio_to_i2c_start(struct gpio_to_i2c_t *handle)
{
	handle->set_output(handle->SCL);
	handle->set_output(handle->SDA);
	handle->output_high(handle->SCL);
	handle->output_high(handle->SDA);
	handle->delay_us(handle->timeout);
	handle->output_low(handle->SDA);
	handle->delay_us(handle->timeout);
	handle->output_low(handle->SCL);
	handle->delay_us(handle->timeout);
}
/**
**** stop condition
*       ---------
*  SCL    
*
*            ----
*  SDA      |
*       ----
*/
static void gpio_to_i2c_stop(struct gpio_to_i2c_t *handle)
{
	handle->set_output(handle->SCL);
	handle->set_output(handle->SDA);
	handle->output_high(handle->SCL);
	handle->output_low(handle->SDA);
	handle->delay_us(handle->timeout);
	handle->output_high(handle->SDA);
	handle->delay_us(handle->timeout);
	// release SDA and SCL
	handle->set_input(handle->SDA);
	handle->set_input(handle->SCL);
}
/**
**** send byte
*                         
*       --       -----       -----       -----       -----       -----       -----       -----       -----                                                                                                   
*  SCL    |     |  1  |     |  2  |     |  3  |     |  4  |     |  5  |     |  6  |     |  7  |     |  8  |                                                                                                       
*          -----       -----       -----       -----       -----       -----       -----       -----       -----                                                                                              
*             ----------- ----------- ----------- ----------- ----------- ----------- ----------- ----------- --
*  SDA       /           X           X           x           x           x           x           x           /
*       ----------------- ----------- ----------- ----------- ----------- ----------- ----------- -----------
*/
static void gpio_to_i2c_send_byte(struct gpio_to_i2c_t *handle,UINT8 byte)
{
	int i;
	handle->set_output(handle->SDA);
	handle->output_low(handle->SCL);
	GPIO_TO_I2C_PRINT("gi2c send byte %02x start\r\n",byte);
	for(i = 0;i < 8;i++) {
		if ((byte) & 0x80) {
			GPIO_TO_I2C_PRINT("gi2c send bit 1\r\n");
			handle->output_high(handle->SDA);
		} else {
			GPIO_TO_I2C_PRINT("gi2c send bit 0\r\n");
			handle->output_low(handle->SDA);
		}
		byte = byte<<1;
		handle->delay_us(handle->timeout);
		handle->output_high(handle->SCL);
		handle->delay_us(handle->timeout);
		handle->output_low(handle->SCL);
		handle->delay_us(handle->timeout);
	}
	GPIO_TO_I2C_PRINT("gi2c send byte %02x end\r\n",byte);
}
static UINT8 gpio_to_i2c_recv_byte(struct gpio_to_i2c_t *handle)
{
	UINT32 i;
	UINT8 value = 0;
	BOOL val;
	handle->set_input(handle->SDA);
	handle->output_low(handle->SCL);
	handle->delay_us(handle->timeout);
	GPIO_TO_I2C_PRINT("gi2c recv byte start\r\n");
	for(i = 0;i < 8;i++) {	
		handle->output_high(handle->SCL);
		handle->delay_us(handle->timeout);
		val = handle->input_read(handle->SDA);
		GPIO_TO_I2C_PRINT("gi2c recv bit %d\r\n",val);
		value = (value << 1) | ( val != 0);
		handle->delay_us(handle->timeout);
		handle->output_low(handle->SCL);
		handle->delay_us(handle->timeout);
	}
	GPIO_TO_I2C_PRINT("gi2c recv byte %02x end\r\n",value);
	return value;
}
/**
**** send ack
*             -----
*  SCL       |     |
*       -----       -----
*            
*  SDA      
*       -----------------
*/
static void gpio_to_i2c_send_ack(struct gpio_to_i2c_t *handle)
{
	handle->set_output(handle->SDA);
	handle->output_low(handle->SCL);
	handle->output_low(handle->SDA);
	handle->delay_us(handle->timeout);
	handle->output_high(handle->SCL);
	handle->delay_us(handle->timeout);
	handle->output_low(handle->SCL);
	handle->delay_us(handle->timeout);
}
/**
**** send nack
*             -----
*  SCL       |     |
*       -----       -----
*        ----------------    
*  SDA  /    
*        
*/
static void gpio_to_i2c_send_nack(struct gpio_to_i2c_t *handle)
{
	handle->set_output(handle->SDA);
	handle->output_low(handle->SCL);
	handle->output_high(handle->SDA);
	handle->delay_us(handle->timeout);
	handle->output_high(handle->SCL);
	handle->delay_us(handle->timeout);
	handle->output_low(handle->SCL);
	handle->delay_us(handle->timeout);
}
static BOOL gpio_to_i2c_recv_ack(struct gpio_to_i2c_t *handle)
{
	BOOL ack;
	handle->set_input(handle->SDA);
	handle->output_high(handle->SCL);
	handle->delay_us(handle->timeout);
	ack = (handle->input_read(handle->SDA) != 0);
	handle->delay_us(handle->timeout);
	handle->output_low(handle->SCL);
	handle->delay_us(handle->timeout);
	return !ack;
}
/*******************************************************************
*  The Msater set SCL and SDA at input to keep high
* 
*  SCL    
*       ---------
*
*       
*  SDA      
*       ---------
********************************************************************/
UINT32 GPIO_TO_I2C_Init(struct gpio_to_i2c_t *handle)
{
	handle->set_input(handle->SDA);
	handle->set_input(handle->SCL);
	if (handle->timeout == 0) {
		handle->timeout = 5;
	}
	handle->delay_us(handle->timeout * 4);
	GPIO_TO_I2C_PRINT("gi2c init\r\n");
	return GPIO_TO_I2C_SetERR(((handle->input_read(handle->SDA) && handle->input_read(handle->SCL)) ? OK_GPIO_TO_I2C : ERR_GPIO_TO_I2C_INIT),0,0,0);
}
/********************************************************************
* The Master is writing multiple bytes to the Slave
* -------------------------------------------------------------------
* |Master| S | SAD + W |   |  RA  |   |  DATA  |   |  DATA  |   | P |
* -------------------------------------------------------------------
* |Slave |   |         |ACK|      |ACK|        |ACK|        |ACK|   |
* -------------------------------------------------------------------
*********************************************************************/
UINT32 GPIO_TO_I2C_SendMulBytes(struct gpio_to_i2c_t *handle,UINT8 SAD,UINT8 RA,UINT8* data,UINT32 size)
{
	UINT32 index = 0;
	GPIO_TO_I2C_PRINT("gi2c start send mul bytes SAD %02x RA %02x data[0] %02x size %ld\r\n",SAD,RA,data[0],size);
	// S
	gpio_to_i2c_start(handle);
	// SAD + W
	gpio_to_i2c_send_byte(handle,I2C_SET_SAD_R_OR_W_FLAG(SAD,I2C_WRITE));
	// ACK
	if (!gpio_to_i2c_recv_ack(handle)) {
		// P
		gpio_to_i2c_stop(handle);
		GPIO_TO_I2C_PRINT("gi2c error send mul bytes, case 'cannot recv ack of write SAD + W'\r\n");
		return GPIO_TO_I2C_SetERR(ERR_GPIO_TO_I2C_NO_W_ACK,SAD,RA,index);
	}
	// RA
	gpio_to_i2c_send_byte(handle,RA);
	// ACK
	if (!gpio_to_i2c_recv_ack(handle)) {
		// P
		gpio_to_i2c_stop(handle);
		GPIO_TO_I2C_PRINT("gi2c error send mul bytes, case 'cannot recv ack of write regiter address'\r\n");
		return GPIO_TO_I2C_SetERR(ERR_GPIO_TO_I2C_NO_W_RA_ACK,SAD,RA,index);
	}
	// DATA
	for(index = 0;index < size;index++) {
		// ACK
		gpio_to_i2c_send_byte(handle,data[index]);
			if (!gpio_to_i2c_recv_ack(handle)) {
				if (index + 1 == size) 
					break;
				// P
				gpio_to_i2c_stop(handle);
				GPIO_TO_I2C_PRINT("gi2c error send mul bytes[%ld], case 'cannot recv ack of write data'\r\n",index);
				return GPIO_TO_I2C_SetERR(ERR_GPIO_TO_I2C_NO_W_RA_ACK,SAD,RA,index);
		}
	}
	// P
	gpio_to_i2c_stop(handle);
	GPIO_TO_I2C_PRINT("gi2c end  send mul bytes SAD %02x RA %02x data[0] %02x size %ld\r\n",SAD,RA,data[0],size);
	return GPIO_TO_I2C_SetERR(OK_GPIO_TO_I2C,SAD,RA,index);
}
/****************************************************************************************
* The Master is receiving multiple bytes of data from the Slave
* ---------------------------------------------------------------------------------------
* |Master| S | SAD + W |   |  RA  |   | Sr | SAD + R |   |        |ACK|        |NACK| P |
* ---------------------------------------------------------------------------------------
* |Slave |   |         |ACK|      |ACK|    |         |ACK|  DATA  |   |  DATA  |    |   |
* ---------------------------------------------------------------------------------------
*****************************************************************************************/
UINT32 GPIO_TO_I2C_RecvMulBytes(struct gpio_to_i2c_t *handle,UINT8 SAD,UINT8 RA,UINT8 *data, UINT32 size)
{
	UINT32 index = 0;
	GPIO_TO_I2C_PRINT("gi2c start recv mul bytes SAD %02x RA %02x data[0] %02x size %ld\r\n",SAD,RA,data[0],size);
	// S
	gpio_to_i2c_start(handle);
	// SAD + W
	gpio_to_i2c_send_byte(handle,I2C_SET_SAD_R_OR_W_FLAG(SAD,I2C_WRITE));
	// ACK
	if (!gpio_to_i2c_recv_ack(handle)) {
		// P
		gpio_to_i2c_stop(handle);
		GPIO_TO_I2C_PRINT("gi2c error recv mul bytes, case 'read SAD + W'\r\n");
		return GPIO_TO_I2C_SetERR(ERR_GPIO_TO_I2C_NO_R_ACK,SAD,RA,index);
	}
	// RA
	gpio_to_i2c_send_byte(handle,RA);
	// ACK
	if (!gpio_to_i2c_recv_ack(handle)) {
		// P
		gpio_to_i2c_stop(handle);
		GPIO_TO_I2C_PRINT("gi2c error recv mul bytes, case 'read regiter address'\r\n");
		return GPIO_TO_I2C_SetERR(ERR_GPIO_TO_I2C_NO_R_RA_ACK,SAD,RA,index);
	}
	// Sr
	gpio_to_i2c_start(handle);
	// SAD + R
	gpio_to_i2c_send_byte(handle,I2C_SET_SAD_R_OR_W_FLAG(SAD,I2C_READ));
	// ACK
	if (!gpio_to_i2c_recv_ack(handle)) {
		// P
		gpio_to_i2c_stop(handle);
		GPIO_TO_I2C_PRINT("gi2c error recv mul bytes, case 'read SAD + R'\r\n");
		return GPIO_TO_I2C_SetERR(ERR_GPIO_TO_I2C_NO_R_ACK2,SAD,RA,index);
	}
	for(index = 0;;) {
		// DATA
		data[index] = gpio_to_i2c_recv_byte(handle);
		if (++index == size) {
			// NACK
			GPIO_TO_I2C_PRINT("gi2c recv mul bytes finish for nack %02x %ld\r\n",RA,size);
			gpio_to_i2c_send_nack(handle);
			break;
		} else {
			// ACK
			GPIO_TO_I2C_PRINT("gi2c recv mul bytes ack\r\n");
			gpio_to_i2c_send_ack(handle);
		}
	}
	gpio_to_i2c_stop(handle);
	GPIO_TO_I2C_PRINT("gi2c end  recv mul bytes SAD %02x RA %02x data[0] %02x size %ld\r\n",SAD,RA,data[0],size);
	return GPIO_TO_I2C_SetERR(OK_GPIO_TO_I2C,SAD,RA,index);
}
