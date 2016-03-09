/* 
 * File:   config.h
 * Author: frans
 *
 * Created on 26 februari 2016, 22:17
 */

#ifndef CONFIG_H
#define	CONFIG_H
 
//int FOSC __attribute__((space(prog), address(0xF80000))) = 0xC703 ; //FRC w/PLL 16x, 30MHz
 int FOSC __attribute__((space(prog), address(0xF80000))) = 0xC70A ;
//int FWDT __attribute__((space(prog), address(0xF80002))) = 0x3F ;   //watchdog timer disabled

#endif	/* CONFIG_H */

