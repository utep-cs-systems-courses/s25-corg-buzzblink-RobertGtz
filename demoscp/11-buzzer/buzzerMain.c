#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

#define BUTTOMS (BIT0 | BIT1 | BIT2 | BIT3)
#define LED_RED BIT0               // P1.0
#define LED_GREEN BIT6             // P1.6
#define LEDS (LED_RED | LED_GREEN)

void led_init();
void led_update();
void state_advance();
void startNote();
void buttom_init();
void switch_interrupt_handler();


int melody[] = {
  392, 349, 330, 294, 262, 247, 220, 196, 
  392, 349, 330, 294, 262, 247, 220, 196, 
  392, 349, 330, 294, 262, 247, 220, 196, 
  392, 349, 330, 294, 262, 247, 220, 196
};

int currentNote = 0;
unsigned char red_on = 1, green_on = 0;
unsigned char led_changed = 0;
volatile char velocityChange = 100;
volatile int isPlaying = 0;


static char redVal[] = {0, LED_RED}, greenVal[] = {0, LED_GREEN};

void led_init() {

  P1DIR |= LEDS;// bits attached to leds are output
  led_changed = 1;
  led_update();
  
}



void led_update() {
  
  if (led_changed) {
    char ledFlags = redVal[red_on] | greenVal[green_on];

    P1OUT &= (0xff^LEDS) | ledFlags; // clear bit for off leds
    P1OUT |= ledFlags;     // set bit for on leds

    led_changed = 0;

  }

}

char toggle_red() {
  
  red_on ^= 1;
  return 1;
  
}

char toggle_green() {

  char changed = 0;
  if (red_on) {
    green_on ^= 1;
    changed = 1;
  }

  return changed;

}





void state_advance() {

  char changed = 0;
  static enum {R=0, G=1} color = G;
  
  if (isPlaying && currentNote < sizeof(melody) / sizeof(melody[0])) {
    switch (color) {
    case R: changed = toggle_red(); color = G; break;
    case G: changed = toggle_green(); color = R; break;

    }
  }

  led_changed = changed;
  led_update();

}

void startNote() {
  
  if (isPlaying && currentNote < sizeof(melody) / sizeof(melody[0])) {
    buzzer_set_period(melody[currentNote] == 0 ? 0 : 2000000 / melody[currentNote]);
    currentNote++; // Move to the next note

  } else {
    currentNote = 0;
    buzzer_set_period(0);
  }
  
}


/*
void
switch_interrupt_handler(){
  
  char p2val = P2IN & BUTTOMS;
  if(!(p2val & BIT0)){
    buzzer_init();
    led_init();
    startNote();
  }
}
*/

void __interrupt_vec(WDT_VECTOR) WDT(){
  static char delay = 0;
  static char currentBlink = 0;
  if(isPlaying){
    if (++delay == velocityChange){
      startNote();
      delay = 0;
    }
    
    if (++currentBlink >= (velocityChange/2)){
      state_advance();
      currentBlink = 0;
    }
  }
}

void buttom_init(){
  P2REN |= BUTTOMS;
  P2IE |= BUTTOMS;
  P2OUT |= BUTTOMS;
  P2DIR &= ~BUTTOMS;
//P2IFG &= ~BUTTOMS;
}
/*
void __interrupt_vec(PORT2_VECTOR) Port2_ISR() {

  if (P2IFG & BUTTOMS) { // Check if any button triggered the interrupt
    // Debounce delay
    __delay_cycles(2000); // 1 ms delay at 2 MHz
    
    if ((P2IN & BUTTOMS) != BUTTOMS) { // Check if any button is still pressed
      // Toggle the melody playback
      isPlaying = !isPlaying;
      if (!isPlaying) {
	buzzer_set_period(0);
	P1OUT &= ~LEDS;
	
      }
    }
    P2IFG &= ~BUTTOMS; // Clear the interrupt flag
  }

}*/

void __interrupt_vec(PORT2_VECTOR) Port2_ISR() {

  if (P2IFG & BIT0) {
    __delay_cycles(2000); // 1 ms delay at 2 MHz
    velocityChange = 100;
    isPlaying = !isPlaying;
    if (!isPlaying) {
      buzzer_set_period(0);
      P1OUT &= ~LEDS;
      
    }
  }
  if (P2IFG & BIT1) {
    // __delay_cycles(2000); // 1 ms delay at 2 MHz
     
    P1OUT &= ~LEDS;
  }
  if(P2IFG & BIT2){
    if (velocityChange > 50) {
      velocityChange -= 10;
    }
  }
  if (P2IFG & BIT3) {
    if (velocityChange < 300) {
      velocityChange += 10;
    }
  }
  P2IFG &= ~BUTTOMS;
}

int main() {
  configureClocks();
  enableWDTInterrupts();
  led_init();
  buzzer_init();
  buttom_init();
  
  or_sr(0x18);          // CPU off, GIE on
}
