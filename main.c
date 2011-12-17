/*
  AVR ATmega168 Analog Comparator Demonstration
  by Mike Szczys
  
  I'm using a voltage divider with a photoresistor on
  PC6 and a voltage divider with equal values on PC7
  to yield a 2.5V reference signal.
  
  The analog comparator is set to throw an interrupt
  whenever there is a zero crossing. I then check the
  ACO bit to see if I should turn on LEDs on Port B or
  not.
  
  jumptuck.com

*/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"

#define T_TIME 		100							//Set half bit frame here (in milliseconds)
#define F_CPU 			1000000						//System clock speed
#define PRESCALE 		1024						//Timer Prescaler
#define T_CLOCK 		(((F_CPU/1000)*T_TIME)/PRESCALE)	//Calculates clock ticks for half bit fram
#define ERROR_MARGIN	T_CLOCK/2					//50% allowance for drift withing half bit frame
#define T_LOW			T_CLOCK-ERROR_MARGIN
#define T_HIGH		T_CLOCK+ERROR_MARGIN
#define DOUBLE_T_LOW		(2*T_CLOCK)-ERROR_MARGIN
#define DOUBLE_T_HIGH		(2*T_CLOCK)+ERROR_MARGIN

volatile unsigned char bit_flag = 0;
volatile unsigned char last_bit = 0;
volatile unsigned int timervalue;
volatile unsigned char sync_lost = 0;

void initComparator(void){
  cli();
  DIDR1 |= (1<<AIN1D) | (1<<AIN0D);	//Disable the digital input buffer
  
  ACSR |= (1<<ACIC);	//Enable analog comparator interrupt
  sei();
}

void initIO(void){
  DDRB = 0xFF;	//PortB as Outputs
  PORTB = 0x00;	//All outputs high
}

void initTimer(void){
  TIMSK1 |= (1<<ICIE1);	//Enable input capture interrupt
  TCCR1B |= (1<<ICNC1);	//Enable the noise canceller
  TCCR1B |= (1<<CS12) | (1<<CS10);	//Prescale of 1024
  
}

int main(void){
  initIO();			//Initialize I/O
  initTimer();
  initComparator();	//Initialize the analog comparator
  lcd_init(LCD_DISP_ON);
  /*
  char buff[7];
  itoa(T_CLOCK,buff,10);
  lcd_puts(buff);
  itoa(ERROR_MARGIN,buff,10);
  lcd_puts(buff);
  */
  
 
  while(1) { 
 
    if (bit_flag) { 
      //lcd_puts("0" + last_bit);	//This math turns a single-digit char into it's ASCII equivalent
      if (last_bit != 0x0F) lcd_putc(last_bit);
      //else lcd_puts("0");
      bit_flag = 0;
    }
    
    if (sync_lost) {
      if (sync_lost == 2) lcd_puts("LOST-2T");
      else lcd_puts("LOST-T");
      lcd_gotoxy(0,1);
      char buff[7];
      itoa(timervalue,buff,10);
      lcd_puts(buff);
      while(1) {}
    }
    
  }
}


ISR (TIMER1_CAPT_vect) {
  static unsigned char sync = 0;					//1 if we have sync, 0 if we don't
  static unsigned char last_was_half_bit_frame = 0;	//1 if last reading was abit, 0 if it was a bitframe transistion
  
  static unsigned char received_byte;				//Stores data until 8 bits are received
  static unsigned char received_bit_count;			//Keeps track of how many bits have been received
	
  unsigned int timestamp = ICR1;	//Read the captured time into a variable
  if (timestamp > T_LOW){
  
    TCNT1 = 0;					//Reset clock for next reading
    
    //if (ACSR & (1<<ACO)) last_bit = 1;	//1 for rising edge, 0 for falling edge
    
    
    unsigned char edge = (TCCR1B & (1<<ICES1))>>ICES1; //Bit shifting to get 1 or 0 based on edge detection setting
    TCCR1B ^= (1<<ICES1);	//Change edge detection for next cycle
    
    //Read in bits if we've found sync
    if (sync) {
      if (last_was_half_bit_frame) {
	//Check if this time was 2T
	if ((timestamp > DOUBLE_T_LOW) && (timestamp < DOUBLE_T_HIGH)) {
	  //This was 2T, this edge is data
	  //last_bit = edge;
	  received_byte = (received_byte<<1) | edge;
	  received_bit_count += 1;
	  last_was_half_bit_frame = 1; 	//This should already be set so this is redundant
	}
	else if ((timestamp > T_LOW) && (timestamp < T_HIGH)) {
	  //This is a bit frame edge and not data
	  last_was_half_bit_frame = 0;
	}
	else {
	  //Error, sync lost!
	  timervalue = timestamp;
	  sync_lost = 2;
	  sync = 0;
	}
	
      }
      else {
	//This time should be T since last time was not
	if ((timestamp > T_LOW) && (timestamp < T_HIGH)) {
	//This was T and last time was a bit frame edge, this edge is data
	  //last_bit = edge;
	  received_byte = (received_byte<<1) | edge;
	  received_bit_count += 1;
	  last_was_half_bit_frame = 1; 	//This should already be set so this is redundant
	}
	else {
	  //Error, sync lost!
	  timervalue = timestamp;
	  sync_lost = 1;
	  sync = 0;
	}
      }
      if (received_bit_count == 8) {
	last_bit = received_byte;
	bit_flag = 1;
	received_byte = 0;
	received_bit_count = 0;
      }
    }
    
    else if ((timestamp > DOUBLE_T_LOW) && (timestamp < DOUBLE_T_HIGH)) {
      //Full bit frame detected. We have Sync!
      //last_bit = edge;	//This is probably deprecated
      
      received_byte = edge;	//Places this data as LSB
      received_bit_count = 1;
      
      //bit_flag = 1;
      last_was_half_bit_frame = 1;
      sync += 1;

    }
  }
}