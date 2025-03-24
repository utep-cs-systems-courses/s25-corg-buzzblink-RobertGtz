#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

#define BUTTOMS (BIT0 | BIT1 | BIT2 | BIT3)

#define LED_RED BIT0               // P1.0

#define LED_GREEN BIT6             // P1.6

#define LEDS (BIT0 | BIT6)

void led_init();

void led_update();


int melody[] = {
  392, 349, 330, 294, 262, 247, 220, 196, 
  392, 349, 330, 294, 262, 247, 220, 196, 
  392, 349, 330, 294, 262, 247, 220, 196, 
  392, 349, 330, 294, 262, 247, 220, 196
};

int currentNote = 0;


extern unsigned char red_on, green_on;

extern unsigned char led_changed;


unsigned char red_on = 1, green_on = 0;

unsigned char led_changed = 0;



static char redVal[] = {0, LED_RED}, greenVal[] = {0, LED_GREEN};





void led_init()

{

  P1DIR |= LEDS;// bits attached to leds are output

  led_changed = 1;

  led_update();

}



void led_update()

{

  if (led_changed) {

    char ledFlags = redVal[red_on] | greenVal[green_on];



    P1OUT &= (0xff^LEDS) | ledFlags; // clear bit for off leds

    P1OUT |= ledFlags;     // set bit for on leds

    led_changed = 0;

  }

}

char toggle_red()/* always toggle! */

{

  static char state = 0;
  if (!(currentNote & 1)){
    red_on ^= 1;

    state = 1;
  }
  
  return 1;/* always changes an led */

}



char toggle_green()/* only toggle green if red is on!  */

{

  char changed = 0;

  if (!(currentNote & 1)) {

    green_on ^= 1;

    changed = 1;

  }

  return changed;

}





void state_advance()/* alternate between toggling red & green */

{

  char changed = 0;



  static enum {R=0, G=1} color = G;
  if (currentNote < sizeof(melody) / sizeof(melody[0])) {
    switch (color) {

    case R: changed = toggle_red(); color = G; break;

    case G: changed = toggle_green(); color = R; break;

    }
  }


  led_changed = changed;

  led_update();

}

void startNote() {
  
  if (currentNote < sizeof(melody) / sizeof(melody[0])) {
    buzzer_set_period(melody[currentNote] == 0 ? 0 : 2000000 / melody[currentNote]);
    currentNote++; // Move to the next note

  } else {
    currentNote = 0;
    buzzer_set_period(0);
  }
  
}
void __interrupt_vec(WDT_VECTOR) WDT(){
  static char delay = 0;
  static char currentBlink = 0;
  if (++delay == 100){
    currentNote++;
    startNote();
    delay = 0;
  }
  if (++currentBlink >= 50){
    state_advance();
    currentBlink = 0;
  }
}

void buttom_init(){
  P2REN |= BUTTOMS;
  P2IE |= BUTTOMS;
  P2OUT |= BUTTOMS;
  P2DIR &= ~BUTTMOS;
}

void switch_inter

int main() {
  configureClocks();
  P1DIR |= LEDS;
  P1OUT &= ~LEDS;
  buttom_init();
  
  buzzer_init();
  
  led_init();
  
  startNote();
  enableWDTInterrupts();
  or_sr(0x18);          // CPU off, GIE on
}
