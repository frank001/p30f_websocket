/* 
 * File:   interrupt.h
 * Author: frans
 *
 * Created on 27 februari 2016, 9:42
 */

#ifndef INTERRUPT_H
#define	INTERRUPT_H

// Function prototype for timer 1 ISR
void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void);

// Function prototype for i2c ISR
void __attribute__((interrupt, no_auto_psv)) _SI2CInterrupt(void);



#endif	/* INTERRUPT_H */

