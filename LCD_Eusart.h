/* 
 * Author: David Cortes
 * Comments: None
 * Revision history: 27/05/2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef LCD_H
#define	LCD_H

#include <xc.h> 
#include <stdio.h> //included for the use of fprint and putch functions.

#ifndef _XTAL_FREQ //ifndef means if not defined yet
#define _XTAL_FREQ 8000000
#endif
#ifndef data
#define data LATD //D port renamed for simplicity
#endif
#ifndef RS
#define RS LATA3 //pin RA3 renamed for simplicity
#endif
#ifndef E
#define E LATA4 //pin RA4 renamed for simplicity
#endif

//Library created just for 4-bit operation mode (space efficient)
void clearDisplay(void);
void returnHome(void);
void writeData(unsigned char);
void writeInstruction(unsigned char);
void initLCD(void);
void enable(void);
void createCharacter(unsigned char [], unsigned char location);
void printMessage(char*);
void printInfo(char* , int);
void printNumber(int);
void putch(unsigned char);
void backlight(unsigned char d);
void firstLineC(char);
void secondLineC(char);
void shiftDRight(void);
void shiftDLeft(void);
void shiftCRight(void);
void shiftCLeft(void);
void deleteChar(void);
void initMessage(void);
void transmision(int,int);

#endif

