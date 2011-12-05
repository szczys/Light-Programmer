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

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char risingCount = 0;

unsigned char T_TIME = 50;
unsigned char low_margin = 39;
unsigned char high_margin = 59;

void initComparator(void){
  cli();
  ACSR |= (1<<ACIE);	//Enable analog comparator interrupt
  sei();
}

void initIO(void){
  DDRB = 0xFF;	//PortB as Outputs
  PORTB = (risingCount<<2);	//All outputs high
}

void initTimer(void){
  TCCR0B |= (1<<CS02) | (1<<CS00);	//Prescale of 1024
  
}

int main(void){
  initIO();			//Initialize I/O
  initTimer();
  initComparator();	//Initialize the analog comparator
   
  while(1){  }
}

//Timer0 ISR
  //Use this timer to throw out duplicate edges

ISR (ANALOG_COMP_vect) {
  //Check for rising or falling edge
  //if (ACSR & (1<<ACO)) PORTB = 0x00;
  //else PORTB = 0xFF;
  
  //Don't read erroneous edges that happen too soon
  if (TCNT0 > low_margin) {
    //reset edge timer
    TCNT0 = 0;
    
    //Count rising edges
    if (ACSR&~(1<<ACO)) {
      risingCount += 1;
      PORTB = (risingCount<<2);
    }
    
    
    ACSR &= ~(1<<ACI);	//Clear Comparator Interrupt Flag
    //Clear Timer0 Interrupt Flag
    
  }
}