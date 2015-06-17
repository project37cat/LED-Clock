// PT6961 display library v3a  // toxcat 2015 copyleft  // HI-TECH C compiler


#define _XTAL_FREQ 8000000 //for delay functions


/* software SPI */

#define DAT      RB3
#define DAT_PIN  TRISB3

#define CLK      RB2
#define CLK_PIN  TRISB2

#define STB      RB1
#define STB_PIN  TRISB1


#define SET_BIT(reg, bit) (reg |= (1<<bit))
#define CLR_BIT(reg, bit) (reg &= (~(1<<bit)))


typedef unsigned char uint8_t; //stdint


char ledbuff[14]; //display buffer


const uint8_t cn=0b0000000; //space
const uint8_t ce=0b1111001; //"E"
const uint8_t c0=0b0111111; //"0"
const uint8_t c1=0b0000110; //"1"
const uint8_t c2=0b1110011; //"2"
const uint8_t c3=0b1110110; //"3"
const uint8_t c4=0b1001110; //"4"
const uint8_t c5=0b1111100; //"5"
const uint8_t c6=0b1111101; //"6"
const uint8_t c7=0b0100110; //"7"
const uint8_t c8=0b1111111; //"8"
const uint8_t c9=0b1111110; //"9"


///////////////////////////////////////////////////////////////////////////////////////////////////
void delay_ms(uint8_t val)
{
while(val--) __delay_ms(1);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void led_data(uint8_t data) //send a byte to the SPI
{
for(uint8_t i=0; i<8; i++)
	{
	if(data & 0x01) DAT=1;
	else DAT=0;
	data>>=0x01;
	CLK=0; //high-low-high
	CLK=1;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void led_comm(uint8_t data) //send a command
{
STB=0; //strobe
led_data(data);
STB=1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void led_init(void)
{
/* init the MCU pins */

DAT_PIN=0;
CLK_PIN=0;
STB_PIN=0;

DAT=0;
CLK=1;
STB=1;

delay_ms(200);

/* PT6961 initial setting */

led_comm(0b01000000); //command 2  //b2 0 - increment address  //b1..b0 00 - data write mode

STB=0;
led_data(0b11000000); //command 3  //b3..b0 - set RAM address 0
for(uint8_t i=0; i<14; i++) led_data(0); //clear the Display RAM (clear screen)
STB=1;

led_comm(0b00000011); //command 1  //b1..b0 display mode (11 - 7 digits, 11 segments)
led_comm(0b10000011); //command 4  //b3 - display ON/OFF  //b2..b0 dimmer (brightness)
led_comm(0b00000011); //command 1
led_comm(0b10001011); //command 4  //display ON
}


///////////////////////////////////////////////////////////////////////////////////////////////////
led_digit(uint8_t pos, uint8_t sign) //print one digit  //position 1..7
{
uint8_t tmp=0;

switch(sign) //select a character code
	{
	case 32: tmp=cn; break; //space
	case 48: tmp=c0; break; //"0"
	case 49: tmp=c1; break; //"1"
	case 50: tmp=c2; break; //"2"
	case 51: tmp=c3; break; //"3"
	case 52: tmp=c4; break; //"4"
	case 53: tmp=c5; break; //"5"
	case 54: tmp=c6; break; //"6"
	case 55: tmp=c7; break; //"7"
	case 56: tmp=c8; break; //"8"
	case 57: tmp=c9; break; //"9"
	default: tmp=ce; break; //"E" if unsupported symbol
	}

for(uint8_t i=0; i<14; i=i+2) //send code in buffer
	{
	if(tmp & 0x01) SET_BIT(ledbuff[i],pos);
	else CLR_BIT(ledbuff[i],pos);
	tmp>>=0x01;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void led_clear(void)
{
for(uint8_t i=0; i<14; i++) ledbuff[i]=0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void led_print(uint8_t pos, const char *str) //print a string  //position 1..7
{
for(;*str;) led_digit(pos++,*str++);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void led_update(void)
{
led_comm(0b01000000); //command 2

STB=0;
led_data(0b11000000); //command 3
for(uint8_t i=0; i<14; i++) led_data(ledbuff[i]); //send buffer to display
STB=1;

led_comm(0b00000011); //command 1
led_comm(0b10001011); //command 4
}
