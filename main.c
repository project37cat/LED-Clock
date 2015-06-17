// LED clock
//
// PIC18LF4320


#include <htc.h>
#include <stdio.h>

#include "led.h"


// configuration bits
__CONFIG(1, RCIO );
__CONFIG(2, BORDIS & WDTDIS );


// bicolor LED
#define LED_PIN  TRISD6=0; TRISD7=0
#define LED_BLU  RD6=0; RD7=1
#define LED_RED  RD6=1; RD7=0
#define LED_OFF  RD6=0; RD7=0

// button
#define BUT_PIN  TRISC7=1
#define BUTTON   RC7
#define BUT_HLD  100
#define BUT_DEB  5


#define TMR0_LOAD 64285
#define TMR1_LOAD 32768

#define LED_DOTS1 ledbuff[3]
#define LED_DOTS2 ledbuff[5]


// flags
bit scrflag; //screen refresh
bit showsec; //show seconds
bit buthold; //hold button

char buff[8]; //string buffer

uint8_t timesec; //time counters
uint8_t timemin;
uint8_t timehrs;

uint8_t setmode; //settings mode

uint8_t butstat; //button state
uint8_t butcnt; //button counter


///////////////////////////////////////////////////////////////////////////////////////////////////
void interrupt handler(void)
{
if(TMR0IF) //timer 0 overflow interrupt //100Hz
	{
	TMR0IF=0;
	TMR0=TMR0_LOAD;
	
	if(BUTTON==0) //if button is pressed
		{
		if(++butcnt>=BUT_HLD)
			{
			butcnt=0;
			butstat=2;
			buthold=1;
			}
		}
	else
		{
		if(butcnt>=BUT_DEB && buthold==0) butstat=1;
		buthold=0;
		butcnt=0;
		}
	}

if(TMR1IF) //timer 1 overflow //1Hz
	{
	TMR1IF=0;
	TMR1=TMR1_LOAD;
	
	scrflag=0;
	
	if(++timesec>59) //seconds
		{
		if(++timemin>59) //minutes
			{
			if(++timehrs>23) timehrs=0; //hours
			timemin=0;
			}
		timesec=0;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void main(void)
{
IRCF2=1; IRCF1=1; IRCF0=1; //8MHz internal RC oscillator

ADCON1=0b1111; //disable analog inputs

//TMR0ON//T08BIT//T0CS//T0SE//PSA//T0PS2:T0PS0
T0CON=0b10010011; //prescaler 011 - 1:16  Fosc/4=2000, 2000/16=125kHz
TMR0=TMR0_LOAD; //preload
TMR0IE=1; //timer0 overflow intterrupt enable

T1CON=0b1111; //T1OSCEN=1; T1SYNC=1; TMR1CS=1; TMR1ON=1; //external 32.768kHz crystal
TMR1=TMR1_LOAD; //preload 32768
TMR1IE=1;  //timer1 overflow intterrupt enable

GIE=1; //global interrupt enable
PEIE=1;

BUT_PIN;
LED_PIN;

led_init();

while(1)
	{
	switch(butstat) //button state
		{
		case 1: //a short press of the button
			scrflag=0;
			butstat=0;
			
			switch(setmode)
				{
				case 0: //show the seconds
					showsec=!showsec;
					break;
					
				case 1: //change the hours
					if(++timehrs>23) timehrs=0;
					break;
					
				case 2: //change the minutes
					if(++timemin>59) timemin=0;
					break;
					
				case 3: //reset seconds
					timesec=0;
					break;
				}
			break;
			
		case 2: //long press
			scrflag=0;
			butstat=0;
			
			if(++setmode>3) setmode=0; //change the settings mode
			
			break;
		}
	
	if(scrflag==0) //refresh display
		{
		led_clear();		
		LED_DOTS1=1;
		
		if(setmode)
			{
			LED_BLU;
			sprintf(buff,"%01u",setmode);
			led_print(1,buff); //print the number of settings mode
			}
		else
			{
			LED_RED;
			}
		
		sprintf(buff,"%02u%02u",timehrs,timemin);	
		led_print(2,buff); //print hours and minutes
		
		if(showsec || setmode)
			{
			sprintf(buff,"%02u",timesec);
			led_print(6,buff); //print seconds	
			LED_DOTS2=1;
			}
		else
			{
			LED_DOTS2=0;
			}
		
		led_update();		
		scrflag=1;
		}
	}
}
