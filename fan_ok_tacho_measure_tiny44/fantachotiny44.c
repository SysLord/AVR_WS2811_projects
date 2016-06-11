/*
 * Fan tacho edge ISR vs a timer. If the ISR wins, the tacho signal was ok and was registered sooner than the timer could fire.
 * Otherwise the timer sets the red led for fan failure.
 * Care must be taken, because the timer needs to be reset to prevent failure triggering.
 * The other way around: If the tacho comes after it is too late we may not set the green led for fan working..
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>


#define DDR_RED   DDRB
#define P_RED     PORTB
#define REDLED    PB1

#define DDR_GREEN DDRB
#define P_GREEN   PORTB
#define GREENLED  PB0


volatile int measure = 0;

/*uint8_t ADC_get()
{
	ADCSRA |= _BV(ADEN) | _BV(ADSC);

	while(!(ADCSRA & (1<<ADIF)));
	return ADCH;   
} */

ISR(ADC_vect)
{
	uint8_t v = ADCH;
	
	// prevents glitches during change.
	// even if no change, the adc value may change, and if OCR0A < TCNT0
	// the next phase will take full counter cycle.
	if (TCNT0 < v)
		OCR0A = v;

	ADCSRA |= (1 << ADSC);
} 

ISR(TIM0_COMPA_vect)
{
	// AD defined timer
	if (measure == 1) {
		// bad = red
		P_RED   |= (1<<REDLED);
		P_GREEN &= ~(1<<GREENLED);
		measure = 0;
	}
}


ISR(INT0_vect) 
{
	// fan tacho
	if (measure == 1) {
		// good = green
		P_RED   &= ~(1<<REDLED);
		P_GREEN |=  (1<<GREENLED);
	}
	
	// restart measurement
	TCNT0 = 0;
	measure = 1;
}


int main() {

	// watchdog
	wdt_enable(WDTO_250MS);

	// led output
	DDR_RED   |= (1<<REDLED);
	DDR_GREEN |= (1<<GREENLED);

	// both on: led test and undefined state
	P_RED   |=  (1<<REDLED);
	P_GREEN |=  (1<<GREENLED);

	// Tacho //////////////////////

	// pull up tacho
	PORTB |= 1<<PB2;

	// interrupt
	// falling edge
	MCUCR |= (1<<ISC01);
	GIMSK |= (1<<INT0);

	// Timer ///////////////////////

	// timer0 8bit
	// enable interrupt
	TIMSK0 |= 1<<OCIE0A;
	// CTC	
	TCCR0A |= (1<<WGM01);

	//prescaleer 1024
	TCCR0B |= _BV(CS00) | _BV(CS02);

	// todo
	OCR0A = 0xFF;

	// left adjust
	ADCSRB |= (1 << ADLAR);

	// division 128
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	// interrupt
	ADCSRA |= (1<<ADIE);
	ADCSRA |= (1 << ADEN);  // Enable ADC
	ADCSRA |= (1 << ADSC);  // Start A2D Conversions 	

	sei();

	while(1)
	{
		// reset watchdog
		wdt_reset();
	}

	return 0;
}
