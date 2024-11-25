#include "HCSR04.h"

unsigned char MeasureDist(void){
  unsigned char aux=0;

  CCP2CON=0b00000100; //set CCP in capture mode with trailing edge
  TMR1=0;             //timer1 set to 0
  CCP2IF=0;           //CCPx flag initialized with 0
  T1CON=0b10010000;   //prescaler of 2 for timer 1 (HCSR04)
  TRIGGER=1;          //ultrasonic sensor is activated
  __delay_us(10);
  TRIGGER=0;
  
 while(ECHO==0);     //waiting for the sensor to reply

  TMR1ON=1;           //timer1, and therefore, time measurement begins
  while(CCP2IF==0 && TMR1IF==0);   //wait for the ultrasonic signal to return
  TMR1ON=0;           //timer1, and therefore, time measurement stops
  if(TMR1IF==1){      
    aux=255;          //if timer1's range excedeed, the measurement gets trunked to 255
    TMR1IF=0;
  }
  else{  
    if(CCPR2>=14732)  //If sensor exceeds 254cm, it gets trunked to this value
      CCPR2=14732;
    aux=CCPR2/58 + 1; //distance measured based on time measurement results
  }
  return aux;         //distance measurement is returned
}
