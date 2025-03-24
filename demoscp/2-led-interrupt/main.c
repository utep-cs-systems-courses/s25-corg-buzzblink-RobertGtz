//Alternate LEDs from Off, Green, and Red
#include <msp430.h>
#include "libTimer.h"
#include "led.h"

volatile int rest = 0;

void
__interrupt_vec(WDT_VECTOR) WDT()	/* 250 interrupts/sec */
{
  static int count = 0;
  if (++count == 125){
    rest = 1;
    count = 0;
  }
} 


int main(void) {
  P1DIR |= LEDS;
  P1OUT &= ~LED_GREEN;
  P1OUT |= LED_RED;

  configureClocks();		/* setup master oscillator, CPU & peripheral clocks */
  enableWDTInterrupts();	/* enable periodic interrupt */

  while (1){
    if(rest){
      rest = 0;
      P1OUT |= LED_GREEN;
      P1OUT &= ~LED_RED;
    }
    or_sr(0x18);
  }
  		/* CPU off, GIE on */
}



