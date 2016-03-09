/* 
 * File:   main.c
 * Author: frans
 *
 * Created on 26 februari 2016, 21:55
 */

#include <stdio.h>
#include <stdlib.h>
#include <p30f4013.h>
#include "config.h"
#include "interrupt.h"
#include "websocket.h"

//extern volatile unsigned char i2c_reg_map[];
//extern char WebSocketKey[];


void delay(unsigned char d) {
    __delay32(d*100000);
}

void I2C_initialize(void) {
    // Configure I2C
    TRISFbits.TRISF2 = 1;
    TRISFbits.TRISF3 = 1;
    I2CCONbits.I2CSIDL = 0 ;
    I2CCONbits.SCLREL = 1 ;
    I2CCONbits.IPMIEN = 0 ;
    I2CCONbits.A10M = 0 ; // 10 bit Address not supported (use of 7 bit)
    I2CCONbits.SMEN = 0 ; // Disable SMBus Compatibility
    I2CCONbits.GCEN = 0 ; // Enable General Call Address (0x00)
    I2CCONbits.STREN = 1 ; // Disable Clock Stretch
    I2CCONbits.ACKDT = 0 ;
    I2CADD = 0x10;
    IPC3bits.SI2CIP = 1 ; // Slave Interrupt Priority
    IPC3bits.MI2CIP = 1 ; // Master Interrupt Priority
    IEC0bits.SI2CIE = 1 ; // Slave Interrupt Enable
    IEC0bits.MI2CIE = 1 ; // Master Interrupt Enable
    IFS0bits.SI2CIF = 0 ; // Clr Slave Interrupt Flag
    IFS0bits.MI2CIF = 0 ; // Clr Master Interrupt Flag
    I2CCONbits.I2CEN = 1 ; // Enable I²C
    I2CSTATbits.I2COV=1;
    
}



unsigned char timeout = 0x3f;

int main(void) {
    unsigned long i;
    unsigned char bitflip=0;
    unsigned char test[] = "810140";
    
    ResetFlags();
    
    //i2c init
    I2C_initialize();
    
    // Configure Timer 1.
    // PR1 and TCKPS are set to call interrupt every 500ms.
    // Period = PR1 * prescaler * Tcy = 58594 * 256 * 33.33ns = 500ms
    T1CON = 0;            // Clear Timer 1 configuration
    T1CONbits.TCKPS = 3;  // Set timer 1 prescaler (0=1:1, 1=1:8, 2=1:64, 3=1:256)
    PR1 = 58594;          // Set Timer 1 period (max value is 65535)
    _T1IP = 1;            // Set Timer 1 interrupt priority
    _T1IF = 0;            // Clear Timer 1 interrupt flag
    _T1IE = 1;            // Enable Timer 1 interrupt
    T1CONbits.TON = 1;    // Turn on Timer 1

    
    TRISB = 0;
    //LATBbits.LATB1= 1;
    
    while(1) {
        _LATB2 = flags.SOCKETCONNECT;
        if (flags.SOCKETCONNECT && !bitflip) {
            //AnswerClient(&test);              //this won't work as rpi2b0 tcp_ip is not improved (it is not listening to the i2c port, only to tcpip)
            
        }
        _LATB0 = 1;
        delay(75);
        _LATB0 = 0;
        delay(75);
    }
    return 0;
}
