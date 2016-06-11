/*
 * Warning: Messy...
 *
 * Created: 23.05.2015 00:20:37
 *  Author: Christian Helmer
 */ 

#define F_CPU 8000000L

#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include "light_ws2812.h"

//#define bit_is_set(var, bit) ((var) & (1 << (bit)))
//#define set_bit(var, bit) ((var) |= (1 << (bit)))
//#define clear_bit(var, bit) ((var) &= ~(1 << (bit)))

#define LEDS 17

struct cRGB led[LEDS];

struct fade { int8_t dir; int8_t slow; int8_t wait; uint8_t c; };

struct cRGB comonColor;

struct fade state[LEDS];


#define C_MAX 255
#define C_UPPER 150
#define C_LOWER 50

// pulsate
#define P_UPPER 100
#define P_LOWER 0
#define P_RANGE 100

//#define C_RANGE (C_UPPER - C_LOWER)

//#define pulse
#undef pulse

//uint8_t dynupper = P_LOWER + P_RANGE;
uint8_t dynlower = P_LOWER;
uint8_t dyndir = 1;

uint16_t dyndelayup = 4;
uint16_t dyndelaydown = -1;
uint16_t dyncnt = 8;
uint16_t dynslow = 8;


#define SLOW_FASTEST 1
/*
uint8_t nextSlow(uint8_t slow) {
	switch(slow) {
		case 1:  return 8;
		break;
		case 2:  return 16;
		break;
		case 4:  return 1;
		break;
		case 8:  return 2;
		break;
		case 16: return 4;
		break;
		default: return 2;
	};
};

uint8_t nextSlowDownwards(uint8_t slow) {
	switch(slow) {
		case 1:  return 16;
		break;
		case 2:  return 16;
		break;
		case 4:  return 8;
		break;
		case 8:  return 2;
		break;
		case 16: return 4;
		break;
		default: return 4;
	};
};
*/
void update() {
	
#ifdef pulse	
	dyncnt--;
	if (dyncnt == 0) {
		dyncnt = dynslow;
		
		dynlower += dyndir;
		
		if (dynlower <= P_LOWER) {
			dyndir = dyndelayup;
		} else if (dynlower >= P_UPPER) {
			dyndir = dyndelaydown;
		}
	}
#endif	
	
	
	for(int l = 0; l < LEDS; l++) {
			
		if ((state[l].wait % state[l].slow) != 0) {
			state[l].wait += 1;
			continue;
		}
		state[l].wait = 1;
			
		if (state[l].dir > 0 && state[l].c >= C_UPPER) {
			state[l].dir = -1;
			//state[l].slow = nextSlowDownwards(state[l].slow);
			} else if (state[l].dir < 0 && state[l].c <= C_LOWER) {
			state[l].dir = 1;
			//state[l].slow = nextSlow(state[l].slow);
		}
			
		//if (state[l].dir > 0)
		//state[l].slow = nextSlow(state[l].slow);
		//else
		//state[l].slow = nextSlowDownwards(state[l].slow);


		// !!!
		state[l].c += state[l].dir;
		//state[l].c = 60;

#ifdef pulse		
		uint16_t f = state[l].c+dynlower;
		led[l].r = ((comonColor.r * f) / (C_UPPER + (P_UPPER/2)));
		led[l].g = ((comonColor.g * f) / (C_UPPER + (P_UPPER/2)));
		led[l].b = 0; // ((comonColor.b * f) / (C_UPPER + (P_UPPER/2)));
		
#else		
		uint16_t f = state[l].c;
		led[l].r = ((comonColor.r * f) / (C_UPPER));
		led[l].g = ((comonColor.g * f) / (C_UPPER));
		led[l].b = ((comonColor.b * f) / (C_UPPER));
#endif		
	}	
}

int main(void)
{
	DDRD |= (1<<PD6);
	DDRD |= (1<<PD5);
	
	PORTB |= (1<<PB0);
	
	_delay_ms(1000);
	
	//orange
	
	comonColor.r=110;
	comonColor.g=30; //40
	comonColor.b=0;
	
	
	//comonColor.r=40;
	//comonColor.g=80;
	//comonColor.b=70;
		
	int i;
	for(i = 0; i < LEDS; i++) {
		state[i].dir = 1;
		state[i].slow = SLOW_FASTEST;
		state[i].wait = 0; 
		//state[i].c = (i * (uint16_t)C_RANGE) / LEDS;
		state[i].c = 0;
	};
	
	state[0].c = 0;
	state[1].c = 230;
	state[2].c = 180;
	state[3].c = 80;
	state[4].c = 200;
	state[5].c = 60;
	state[6].c = 255;
	state[7].c = 90;
	state[8].c = 10;
	state[9].c = 233;
	state[10].c = 130;
	state[11].c = 40;	
	state[12].c = 240;
	
	
	state[0].slow = 15;
	state[1].slow = 5;
	state[2].slow = 3;
	state[3].slow = 4;
	state[4].slow = 6;
	state[5].slow = 9;
	state[6].slow = 10;
	state[7].slow = 2;
	state[8].slow = 13;
	state[9].slow = 4;
	state[10].slow = 8;
	state[11].slow = 6;	
	state[12].slow = 12;

	uint8_t x = 1;
	uint8_t a = 0;
	while(1) {		
		/*
		if (a==0) {
			a=1;
						comonColor.r=40;
						comonColor.g=80;
						comonColor.b=70;
		} else {
			a=0;
			comonColor.r=110;
			comonColor.g=40;
			comonColor.b=0;
		}
		
		if (x==0 && a==0) {
			comonColor.r=40;
			comonColor.g=80;
			comonColor.b=70;
			a=1;
		} else if (x==0 && a==1) {
			comonColor.r=110;
			comonColor.g=40;
			comonColor.b=0;
			a=0;
		}
		x++;
		*/
		
		if ((PINB & (1<<PB0))) {
			comonColor.r=40;
			comonColor.g=80;
			comonColor.b=70;			
		} else {
				comonColor.r=110;
				comonColor.g=30; //40
				comonColor.b=0;		
		}
		
		update();
		ws2812_setleds_pin(led, LEDS, _BV(6));
		_delay_ms(2);
    }
}