/*
 * 6x6 ws2812 LED array with cursor navigation.
 * 
 * Created: 23.05.2015 00:20:37
 * Author: Christian Helmer
 */ 

#define F_CPU 8000000L

#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include "light_ws2812.h"

#define MAX(a,b) (a>b ? a : b)
#define MIN(a,b) (a>b ? b : a)

#define LEDS 36

struct cRGB ledstate[LEDS];

// cursor
uint8_t posx = 0;
uint8_t posy = 0;
uint8_t lastx = 0;
uint8_t lasty = 0;

// beides zusammen ist net so toll
#undef FLIMMER
#define LINEFLIMMER

#ifdef FLIMMER
// flimmerfactor
int8_t ff = 0 ;
// richtung
uint8_t fr = 1 ;
#endif

#ifdef LINEFLIMMER
uint8_t flimmerline = 0;
#endif


void update() {
	
	struct cRGB *last = &ledstate[lastx+(lasty*6)];
	last->r=0;
	last->g=10;
	last->b=0;
	
	lastx=posx;
	lasty=posy;

#ifdef FLIMMER	
	if (ff > 1) {
		fr = -1;
	}
	if (ff <= 1) {
		fr = 1;
	}	
	ff = ff + fr;
#endif
#ifdef LINEFLIMMER
	flimmerline = (flimmerline + 1) % 6;
#endif	
	
	for(uint8_t i = 0; i < LEDS; i++) {
		struct cRGB *f = &ledstate[i];
		if (f->r < 5) {
			f->r=0;			
			f->b=0;	
			
			uint8_t val = 5;
#ifdef FLIMMER
			val += (ff  * (((i+5)%2==1)  + ((i+3)%4==0) - ((i+1)%3==1)));
#endif			
#ifdef LINEFLIMMER
			if (flimmerline == (i / 6)) {
				val += 1;
				//f->r=1;
			}	
#endif		
			f ->g = val;	
			
		}
	}
	
	struct cRGB *cursor = &ledstate[lastx+(lasty*6)];
	cursor->r=10;
	cursor->g=100;
	cursor->b=50;
}

int main(void)
{
	// cursor pull up, no output
	PORTB  |= (1<<PB0);
	PORTB  |= (1<<PB1);
	PORTB  |= (1<<PB2);
	PORTB |= (1<<PB3);
	
	// ws2812 led serial output
	DDRD |= (1<<PD6);
	
	_delay_ms(1000);

	for(uint8_t i = 0; i < LEDS; i++) {
		struct cRGB *f = &ledstate[i];
		f->r=0;
		f->g=10;
		f->b=0;
	}

	uint8_t changes = 1;
	uint8_t debounce = 0;
	while(1) {		
		_delay_ms(25);
		

		if (0==(PINB & (1<<PB0))) {	
			if (debounce > 5) {
				posx=MIN(5,(posx+1)) % 6;
				changes=1;
				debounce = 0;
			}
			debounce += 1;
		}
		if (0==(PINB & (1<<PB1))) {
			if (debounce > 5){
				posx= MAX(0,(posx-1)) % 6;
				changes=1;
				debounce = 0;
			}
			debounce += 1;
		}
		if (0==(PINB & (1<<PB2))) {
			if (debounce > 5){
				posy=MIN(5,(posy+1)) % 6;
				changes=1;
				debounce = 0;
			}
			debounce += 1;
		}
		if (0==(PINB & (1<<PB3))) {
			if (debounce > 5){
				posy= MAX(0,(posy-1)) % 6;
				changes=1;
				debounce = 0;
			}
			debounce += 1;
		}
				
		//if (changes) {
			changes = 0;
			update();
			ws2812_setleds_pin(ledstate, LEDS, _BV(6));
			//_delay_ms(10);
		//}
    }
}