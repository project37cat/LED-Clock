// LED Clock  (PIC18LF4320 + PT6961)
// Compiler: HI-TECH C PRO for the PIC18 MCU Family  V9.63PL3
// 21-oct-15 Copyleft toxcat


#include <htc.h>
#include <stdio.h>

#include "led.h"


// configuration bits
__CONFIG(1, RCIO );  //internall RC oscillator
__CONFIG(2, BORDIS & WDTDIS );  //disable Brown-out Reset  //disable  Watchdog Timer


typedef unsigned char uint8_t; //stdint


// bicolor LED
#define LED_PIN  {TRISD6=0; TRISD7=0;}
#define LED_BLU  {RD6=0; RD7=1;}
#define LED_RED  {RD6=1; RD7=0;}
#define LED_OFF  {RD6=0; RD7=0;}

// button
#define BUT_PIN  TRISC7=1
#define BUTTON   RC7
#define BUT_HLD  100
#define BUT_DEB  5

// active buzzer
#define BUZZ_PIN  TRISD5=0
#define BUZZER    RD5

#define TMR0_LOAD 64286
//#define TMR1_LOAD 32768
#define TMR1_LOAD 32770

#define LED_DOTS1 ledbuff[3]
#define LED_DOTS2 ledbuff[5]


char buff[8]; //string buffer

uint8_t timesec; //time counters
uint8_t timemin;
uint8_t timehrs;

uint8_t setmode; //settings mode

bit     scrflag; //screen refresh flag //0 - need refresh
bit     showsec; //show seconds flag //0 - show seconds

uint8_t blinkcnt; //counter for blink
bit     blinkstart; //start counter flag
bit     blinkflag; //flag for blink

bit     buthold; //button hold flag
uint8_t butstat; //button state //2 - long press, 1 - short press, 0 - not pressed
uint8_t butcnt; //counter for button

uint8_t buzzcnt; //counter for buzzer

bit     alarmenable; //alarm clock turned on
bit     alarmactive; //alarm signal is active
uint8_t alrmhrs; //alarm time
uint8_t alrmmin;


///////////////////////////////////////////////////////////////////////////////////////////////////
void interrupt handler(void)
{
/*****************************************************************************/
if(TMR0IF) //timer 0 overflow interrupt //100Hz
	{
	TMR0IF=0;
	TMR0=TMR0_LOAD;
	
	if(blinkstart)
		{
		if(++blinkcnt>=50)
			{
			blinkcnt=0;
			blinkstart=0;
			blinkflag=0;
			scrflag=0;
			}
		}	
	
	if(BUTTON==0) //if button is pressed
		{
		if(++butcnt>=BUT_HLD)
			{
			butcnt=0;
			butstat=2; //long press
			buthold=1;
			}
		}
	else
		{
		if(butcnt>=BUT_DEB && buthold==0) butstat=1; //short press
		buthold=0;
		butcnt=0;
		}
	
	if(alarmenable && timehrs==alrmhrs && timemin==alrmmin) alarmactive=1; //alarm
	
	if(alarmactive) //alarm signal
		{
		if(BUZZER==0 && buzzcnt==0) BUZZER=1;
			buzzcnt++;
			if(buzzcnt==30) BUZZER=0;
			if(buzzcnt==80) buzzcnt=0;
		
		if(alarmenable && butstat) //disable alarm
			{
			BUZZER=0;
			buzzcnt=0;
			alarmenable=0;
			alarmactive=0;
			butstat=0;
			scrflag=0;
			}
		}
	}

/*****************************************************************************/
if(TMR1IF) //timer 1 overflow //1Hz
	{
	TMR1IF=0;
	TMR1=TMR1_LOAD;
	
	scrflag=0;
	
	blinkstart=1;
	
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

/* init. pins */
ADCON1=0b1111; //disable analog inputs
BUT_PIN;
LED_PIN;
LED_OFF;
BUZZ_PIN;
BUZZER=0;

//TMR0ON//T08BIT//T0CS//T0SE//PSA//T0PS2:T0PS0
T0CON=0b10010011; //prescaler 011 - 1:16  Fosc/4=2000, 2000/16=125kHz
TMR0=TMR0_LOAD; //preload  //65536-64286=1250 125k/1250=100Hz
TMR0IE=1; //timer0 overflow intterrupt enable

T1CON=0b1111; //T1OSCEN=1; T1SYNC=1; TMR1CS=1; TMR1ON=1; //external 32.768kHz crystal
TMR1=TMR1_LOAD; //preload  //65536-32768=32768, 32.768kHz/32768=1Hz
TMR1IE=1; //timer1 overflow intterrupt enable

GIE=1; //enable interrupts
PEIE=1;

led_init();

while(1)
	{
	switch(butstat) //check the button state
		{
		case 1: //a short press of the button
			scrflag=0;
			butstat=0;
			blinkflag=0;
			
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
					
				case 4: //change the brightness
					if(++leddimm>3) leddimm=0;
					break;
										
				case 5: //set alarm
					if(++alrmhrs>23) alrmhrs=0;
					break;
				
				case 6: //set alarm
					if(++alrmmin>59) alrmmin=0;
					break;
				
				case 7: //alarm on/off
					alarmenable=!alarmenable;
					break;
				}
			break;
			
		case 2: //long press
			scrflag=0;
			butstat=0;	
			blinkflag=1;
			
			if(++setmode>7) setmode=0; //change the settings mode
			break;
		}
	
	if(scrflag==0) //refresh the screen
		{
		led_clear();
		
		if(setmode)
			{
			LED_BLU;
			ledbuff[11]=1; //settings mode indicator
			sprintf(buff,"%01u",setmode);
			led_print(1,buff); //print the number of settings mode
			}
		else LED_RED;
		
		switch(setmode)
			{
			/*****************************************************************************/
			case 0: //screen in normal mode
				sprintf(buff,"%02u%02u",timehrs,timemin);
				led_print(2,buff); //print hours and minutes
				
				if(!showsec) //show seconds
					{
					LED_DOTS1=1; //dots 1 ON
					LED_DOTS2=1; //dots 2 ON
					sprintf(buff,"%02u",timesec);
					led_print(6,buff); //print seconds
					}
				else
					if(!blinkflag) //blink dots 1
						{
						LED_DOTS1=1; //dots 1 ON
						blinkflag=1;
						}			
				break;
			/*****************************************************************************/			
			case 1:	//settings mode - 1, set hours
				sprintf(buff,"%02u%02u",timemin,timesec);	
				led_print(4,buff);
				
				if(!blinkflag) //blink hours
					{			
					sprintf(buff,"%02u",timehrs);	
					led_print(2,buff); //print hours
					blinkflag=1;
					}
					
				LED_DOTS1=1; //dots 1 ON
				LED_DOTS2=1; //dots 2 ON
				break;
			/*****************************************************************************/	
			case 2: //settings mode - 2, set minutes
				sprintf(buff,"%02u  %02u",timehrs,timesec);	
				led_print(2,buff);
				
				if(!blinkflag) //blink min.
					{			
					sprintf(buff,"%02u",timemin);	
					led_print(4,buff); //print min.
					blinkflag=1;
					}
					
				LED_DOTS1=1; //dots 1 ON
				LED_DOTS2=1; //dots 2 ON	
				break;
			/*****************************************************************************/
			case 3: //settings mode - 3, set seconds
				sprintf(buff,"%02u%02u",timemin,timehrs);	
				led_print(2,buff);
				
				if(!blinkflag) //blink sec.
					{			
					sprintf(buff,"%02u",timesec);	
					led_print(6,buff); //print sec.
					blinkflag=1;
					}
					
				LED_DOTS1=1; //dots 1 ON
				LED_DOTS2=1; //dots 2 ON	
				break;
			/*****************************************************************************/								
			case 4: //settings mode - 4, set brightness
				sprintf(buff,"%01u",leddimm);
				led_print(7,buff);
				break;
			/*****************************************************************************/		
			case 5: //settings mode - 5, set alarm hours
				sprintf(buff,"%02u",alrmmin); //print minutes
				led_print(6,buff);
				
				if(!blinkflag) //blink hours
					{			
					sprintf(buff,"%02u",alrmhrs);	
					led_print(4,buff); //print hours
					blinkflag=1;
					}
				
				LED_DOTS2=1;
				break;
			/*****************************************************************************/
			case 6: //settings mode - 6, set alarm minutes
				sprintf(buff,"%02u",alrmhrs); //print hours
				led_print(4,buff);
				
				if(!blinkflag) //blink min.
					{			
					sprintf(buff,"%02u",alrmmin);	
					led_print(6,buff); //print min.
					blinkflag=1;
					}
				
				LED_DOTS2=1;
				break;
			/*****************************************************************************/
			case 7: //settings mode - 7, set alarm ON/OFF
				sprintf(buff,"%02u%02u",alrmhrs,alrmmin);
				led_print(4,buff);
				LED_DOTS2=1;
				break;
				}
		
		if(alarmenable) ledbuff[0]|=0x01; //alarm indicaror
		
		led_update();
		scrflag=1;
		}
	}
}
