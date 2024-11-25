//this header file contains the definitions needed to support the use of the HCSR04 ultrasonic sensor 
#ifndef HCSR04_H
#define	HCSR04_H

#include <xc.h>  
#include<stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _XTAL_FREQ //ifndef means if not defined yet
#define _XTAL_FREQ 8000000 //sampling frequency must be 8MHz for the ultrasonic sensor to work properly
#endif

#ifndef	TRIGGER
#define TRIGGER RC0 //trigger assigned to RC0 pin
#endif
#ifndef ECHO
#define ECHO RC1 //echo assigned to RC1 pin
#endif 
unsigned char MeasureDist(void); //definition of function to measure distance with the ultrasonic sensor
#endif	/* XC_HEADER_TEMPLATE_H */

