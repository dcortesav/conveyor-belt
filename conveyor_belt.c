//Project developed by David Santiago Cortes Avila and Brayan David Quintero

#include <xc.h>
#define _XTAL_FREQ 8000000
#include "LCD_Eusart.h"
#include "HCSR04.h"
#pragma config FOSC = INTOSC_EC
#pragma config WDT = OFF
#pragma config LVP = OFF

//Variables
unsigned char user_chosen_value;
unsigned char counter_value;
unsigned int potenciometer_measure;
unsigned char serial_communication_enabler_flag = 0;
unsigned char distance_measure;
unsigned int pwm_value=0;

//Auxiliary variables
unsigned char state_machine_value = 1;
unsigned char pressed_key;
unsigned char digit_number_register;

//Interrupt handling
void __interrupt() ISR(void);

//Functions for modeling conveyor belt's behavior
void state_1(void);
void state_2(void);
void state_3(void);
void debouncing(void);
void assignRGB(unsigned char);
unsigned char conversion_ADC(unsigned char);

//main
void main (void){
    
    //internal oscillator configuration set to 8MHz
    OSCCON = 0b01110010;
    __delay_ms(10);
   
    //ADC module configuration
    ADCON0 = 1; //channel AN0 configuration and module power on
    ADCON1 = 14; //analog port assignment and configuration of + and - references
    ADCON2 = 0b01010001; //format, acquisition time and module clock configuration
    
    //EUSART module configuration
    TXSTA = 0b00100100; //enable the transmitter and high speed mode
    RCSTA = 0b10010000; //enable serial ports and receiver
    BAUDCON = 0b00001000; //enable 16-bit divisor for baud rate calculation
    SPBRG = 207; //configuration of the register that controls the baud rate
    
    //port configuration as inputs or outputs
    TRISA = 0b11100001;
    TRISB = 0b11110000;
    TRISC2 = 0;
    TRISC0 = 0;
    TRISD = 0;
    TRISE = 0b11111000;
    
    //port cleaning
    LATA = 0b00000100;
    LATB = 0;
    LATC2 = 0;
    LATD = 0;
    LATE = 0;
    
    //enable pull-up resistors of port B
    RBPU = 0;
    __delay_ms(100);
    
    //timer0 interrupt configuration
    T0CON = 0b00000100; //divisor set to 32
    TMR0 = 3036; //prescaler configuration
    TMR0IF = 0;
    TMR0IE = 1;
    
    //port B interrupt configuration
    RBIF = 0;
    RBIE = 1;
    
    //receiver interrupt configuration
    RCIE = 1;
    RCIF = 0;
    
    //general interrupt configuration
    GIE = 1; //enable interruptions
    PEIE = 1; //enable peripheral interrupts
    TMR0ON = 1; //timer0 power on
       
    //CCP and timer 2 configuration
    PR2 = 255;    //PWM frequency set to 100khz
    CCPR1L = 0;   //duty cycle configuration 
    T2CON = 0b00000011; //preescaler set to 16
    TMR2 = 0;     //timer2 interrupt flag is initialized
    TMR2ON = 1; //timer2 power in
    CCP1CON = 0b00001100; //CCP configured in PWM mode
    
    //LCD initialization routine
    initLCD(); //approx. 27.36ms
   
    //welcoming message LCD
    initMessage(); //approx. 3326.4ms
    __delay_ms(1673.6); //delay for the welcoming message to last approx. 5s
    
    //main loop
    while(1){
        if(state_machine_value == 1) state_1();
        else if(state_machine_value == 2) state_2();
        else if(state_machine_value == 3) state_3();
       
    }
}

//Interrupt handling
void __interrupt() ISR(void){
    if(TMR0IF == 1){
        TMR0IF = 0;
        TMR0 = 3036;
        LATA1 ^= 1;
        potenciometer_measure = conversion_ADC(0); 
        
        distance_measure = MeasureDist();
        transmision(pwm_value,distance_measure);
        if(!serial_communication_enabler_flag){
        CCPR1L=potenciometer_measure; 
        pwm_value=(potenciometer_measure*100/255);
       }
        else{
        pwm_value=(CCPR1L*100/255); 
        }
    }else if(RCIF){
        RCIF = 0;
        if(RCREG == 'z' || RCREG == 'Z'){
            serial_communication_enabler_flag = 1;
            CCPR1L= 0;
        }
        else if(RCREG == 'x' || RCREG == 'X'){
            serial_communication_enabler_flag = 1;
            CCPR1L= 51 ; //20%
        }
        else if(RCREG == 'c' || RCREG == 'C'){
            serial_communication_enabler_flag = 1;
            CCPR1L= 102 ; //40%
        } 
        else if(RCREG == 'v' || RCREG == 'V'){
            serial_communication_enabler_flag = 1;
            CCPR1L= 153 ; //60%
        }
        else if(RCREG == 'b' || RCREG == 'B'){
            serial_communication_enabler_flag = 1;
            CCPR1L= 204 ; //80%
        } 
        else if(RCREG == 'n' || RCREG == 'N'){
            serial_communication_enabler_flag = 1;
            CCPR1L= 255 ; //100%
        }
        else if(RCREG == 'e' || RCREG == 'E'){
            LATE = 0b11111100;
            CCPR1L = 0;
            clearDisplay();
            printMessage("    Emergency   ");
            secondLineC(0);
            printMessage("      Stop      ");
            TMR0ON = 0; //disable timer0
            CREN = 0; //disable receiver
            while(1); //conveyor belt blocked until hard reset
        }else if(RCREG == 'r' || RCREG == 'R'){
            if(state_machine_value == 2){
                counter_value = 0;
                LATD = 0;
                LATE = 0b00000101;
                secondLineC(9);
                if(user_chosen_value < 10){
                    printNumber(0);
                    printNumber(user_chosen_value);
                }
                else printNumber(user_chosen_value);
            }
        }
    }else if(RBIF == 1){
        if(PORTB != 0b11110000){
            pressed_key = 100;
            LATB = 0b11111110;
            if(RB4==0){
                debouncing();
                pressed_key = 1;
            }
            else if(RB5==0){
                debouncing();
                pressed_key = 2;
            }
            else if(RB6==0){
                debouncing();
                pressed_key = 3;
            }
            else if(RB7==0){ //emergency stop
                LATE = 0b11111100;
                CCPR1L=0;
                clearDisplay();
                printMessage("    Emergency   ");
                secondLineC(0);
                printMessage("      Stop      ");
                TMR0ON = 0; //disable timer0
                CREN = 0; //disable receiver
                while(1); //conveyor belt blocked until hard reset
            }
            else{
                LATB = 0b11111101;
                if(RB4==0){
                    debouncing();
                    pressed_key = 4;
                }
                else if(RB5==0){
                    debouncing();
                    pressed_key = 5;
                }
                else if(RB6==0){
                    debouncing();
                    pressed_key = 6;
                }
                else if(RB7==0){ //end process
                    debouncing();
                    if(state_machine_value == 2){
                        state_machine_value = 3;
                        LATD = user_chosen_value%10;
                        assignRGB(user_chosen_value);
                    }
                }
                else{
                    LATB = 0b11111011;
                    if(RB4==0){
                        debouncing();
                        pressed_key = 7;
                    }
                    else if(RB5==0){
                        debouncing();
                        pressed_key = 8;
                    }
                    else if(RB6==0){
                        debouncing();
                        pressed_key = 9;
                    }
                    else if(RB7==0){ //reset
                        debouncing();
                        if(state_machine_value == 2){
                            counter_value = 0;
                            LATD = 0;
                            LATE = 0b00000101;
                            secondLineC(9);
                            if(user_chosen_value < 10){
                                printNumber(0);
                                printNumber(user_chosen_value);
                            }
                            else printNumber(user_chosen_value);
                        }
                    }
                    else{
                        LATB = 0b11110111;
                        if(RB4==0){//backlight control
                            debouncing();
                            LATA2 ^= 1;
                        }
                        else if(RB5==0){
                            debouncing();
                            pressed_key = 0;
                        }
                        else if(RB6==0){ //"ok" key
                            debouncing();
                            pressed_key = 10;
                        }
                        else if(RB7==0){ //"delete" key
                            debouncing();
                            if(state_machine_value == 1 && digit_number_register > 0){
                                deleteChar();
                                digit_number_register -= 1;
                            }
                        }
                    }
                }
            }
        }
        LATB = 0b11110000;
        RBIF = 0;
    }
}

//Functions for modeling conveyor belt's behavior
void state_1(void){
    pressed_key = 100;
    digit_number_register = 0;
    clearDisplay();
    printMessage("# of Pieces:");
    secondLineC(0);
    while(state_machine_value == 1){
        while(pressed_key==100);
        if(pressed_key < 10 && digit_number_register == 0){
            user_chosen_value = pressed_key;
            printNumber(pressed_key);
            digit_number_register +=1;
        }else if(pressed_key < 10 && digit_number_register == 1){
            if(user_chosen_value < 6){
                user_chosen_value = 10*user_chosen_value + pressed_key;
                printNumber(pressed_key);
                digit_number_register += 1;
            }else if(user_chosen_value > 9){
                user_chosen_value = (user_chosen_value - (user_chosen_value%10)) + pressed_key;
                deleteChar();
                printNumber(user_chosen_value);
                digit_number_register += 1;
            }
        }else if(pressed_key==10 && digit_number_register > 0){
            state_machine_value = 2;
        }
        pressed_key = 100;
    }
}

void state_2(void){
    counter_value = 0;
    LATD = 0;
    LATE = 0b00000101;
    clearDisplay();
    if(user_chosen_value < 10){
        printInfo("Objective", 0);
        printNumber(user_chosen_value);
        secondLineC(0);
        printInfo("Missing",0);
        printNumber(user_chosen_value);
    }else{
        printInfo("Objective", user_chosen_value);
        secondLineC(0);
        printInfo("Missing", user_chosen_value);
    }
    while((user_chosen_value - counter_value) > 0){
        while(distance_measure<4 || distance_measure>8 && state_machine_value == 2);
        if(state_machine_value != 2) break;
        __delay_ms(1500);
        if(counter_value%10 < 9){
            counter_value += 1;
        }else{
            counter_value += 1;
            assignRGB(counter_value);
        }
        LATD = counter_value%10;
        secondLineC(9);
        if(user_chosen_value-counter_value < 10){
            printNumber(0);
            printNumber(user_chosen_value - counter_value);
        }
        else printNumber(user_chosen_value - counter_value);
    }
    if(state_machine_value == 1) return;
    state_machine_value = 3;
}

void state_3(void){
    pressed_key = 100;
    clearDisplay();
    printMessage("    Countdown   ");
    secondLineC(0);
    printMessage("    Finalized   ");
    while(pressed_key != 10);
    state_machine_value = 1;
}

void debouncing(void){
    __delay_ms(200);
}

void assignRGB(unsigned char number){
    switch((number%100 - number%10)){
        case 50:
            LATE = 0b00000111;
            break;
        case 40:
            LATE = 0b00000110;
            break;
        case 30:
            LATE = 0b00000010;
            break;
        case 20:
            LATE = 0b00000011;
            break;
        case 10:
            LATE = 0b00000001;
            break;
        default:
            LATE = 0b00000101;
    }
}

unsigned char conversion_ADC(unsigned char canal){
    ADCON0 = (canal<<2) | 0b00000001;
    GO = 1;
    while(GO == 1);
    return ADRESH;
}