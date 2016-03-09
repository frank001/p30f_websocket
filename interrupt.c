#include <p30f4013.h>
#include <i2c.h>
#include "interrupt.h"
#include "websocket.h"

//extern char WebSocketKey[];

volatile unsigned int i2c_byte_count;
volatile unsigned int i2c_reg_addr;
volatile unsigned char i2c_reg_map[255];       //TODO: make this as small as possible
unsigned char address_flag;                     //TODO: create a flag register/struct
unsigned char data_flag;
unsigned char rcv;

// Timer 1 interrupt service routine
void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void) {
    // Clear Timer 1 interrupt flag
    _T1IF = 0;
     // Toggle LED on RB1
    _LATB1 = 1 - _LATB1;
}

void __attribute__((interrupt, no_auto_psv)) _SI2CInterrupt(void) {
    unsigned char data;
    unsigned char tmp;
    unsigned char i;
    
    IEC0bits.SI2CIE = 0; //Disable interrupts
    //LATBbits.LATB2 = 1;
	if((I2CSTATbits.R_W == 0)&&(I2CSTATbits.D_A == 0)) {	//address matched, writing register address to slave
		tmp = I2CRCV;                               //dummy read to remove stored address value
		address_flag = 1;                           //next byte will be address
		data_flag = 0;
	}
	else if((I2CSTATbits.R_W == 0)&&(I2CSTATbits.D_A == 1)) {	//Check for data	
		if(address_flag) {
			address_flag = 0;	
			data_flag = 1;                          //next byte is data
            i2c_reg_addr = I2CRCV;                  //store address
			I2CCONbits.SCLREL = 1;                  //release SCL line
		} else 
            if(data_flag) {
            rcv = I2CRCV;
            ReadClient(rcv);                     //pass data to websocket
            i2c_reg_map[i2c_reg_addr++] = rcv;   //store data into register, auto-increment
			I2CCONbits.SCLREL = 1;               //release SCL line
            
		}
	}
	else if((I2CSTATbits.R_W == 1)&&(I2CSTATbits.D_A == 0)) { //Been told to put data on bus, must already have desired register address
		tmp = I2CRCV;                               //ditch stored address
		I2CTRN = i2c_reg_map[i2c_reg_addr++];       //read data from register & send data to I2C transmit buffer, setting D/A in the process
		I2CCONbits.SCLREL = 1;                      //release SCL line
	} else 
        if((I2CSTATbits.R_W == 1)&&(I2CSTATbits.D_A == 1)) { //D/A hasn't been reset, master must want more data
		I2CTRN = i2c_reg_map[i2c_reg_addr++];       //read data from register & send it to I2C master device
		I2CCONbits.SCLREL = 1;                      //release SCL line
	}
	
    i2c_reg_addr %= sizeof(i2c_reg_map);            //limit address to size of register map
    
    _SI2CIF = 0;                                    //clear I2C1 Slave interrupt flag
    //LATBbits.LATB2 = 0;
	IEC0bits.SI2CIE = 1;                            //Enable interrupts
 
} 
