/*
 * Code for using an old hard disk motor as a rotary encoder.
 *
 * This project is inspired by and originally based on the rotary encoder project
 * by Franck Fleurey which can be found here:
 *
 * http://www.fleurey.com/franck/pmwiki.php?n=Main.HDDEncoder
 *
 * The intention of this project is a little different from the original approach,
 * though. The ATtiny running this code is supposed to output a pulse on one of two
 * output pins (PB3/PB4) whenever an increment or decrement has been detected. These
 * output pins are in turn monitored by another MCU, so you could call this a rotary
 * encoder driver board for interfacing an MCU with an HDD motor.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// the output ports on which a pulse will be sent for when the encoder increments or decrements
#define OUT_DEC PB3
#define OUT_INC PB4

// the amount of samples that have to have the same direction in a row in order to make the pulse count.
#define SMOOTH_COUNT 2

// the output pulse width in milliseconds
#define PULSE_WIDTH 2
#define PULSE_DISTANCE 1

void pulse_out(uint8_t port) {
	PORTB |= _BV(port);
	_delay_ms(PULSE_WIDTH);
	PORTB &= ~_BV(port);
	_delay_ms(PULSE_DISTANCE);
}

int main(void)
{
	// ADC off
	ADCSRA &= ~_BV(ADEN);

	// set LED pins to output
	DDRB |= _BV(PB3) | _BV(PB4);
	PORTB &= ~(_BV(PB3) | _BV(PB4));

	// set input pins to input
	DDRB &= ~(_BV(PB0)|_BV(PB1)|_BV(PB2));

	GIMSK = 1<<INT0;							// Enable INT0
	MCUCR = 1<<ISC01 | 1<<ISC00;	// Rising edge

	sei();

	while(1) {
		// we make the MCU sleep in order to consume less power - round about 1.45mA instead of 2.15mA in my test setup.
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}

	return 0;
}

#define DIR_UP 1
#define DIR_DOWN 2
volatile int direction = 0;
volatile int pulses = 0;

ISR(INT0_vect) {
	int val =  (PINB & 0x03);

	if(val == 1) {
		if(direction == DIR_DOWN) {
			pulses++;
			if(pulses >= SMOOTH_COUNT) {
				pulse_out(OUT_DEC);
				pulses = 0;
			}
		} else {
			direction = DIR_DOWN;
			pulses = 0;
		}
	}
	else if(val == 3) {
		if(direction == DIR_UP) {
			pulses++;
			if(pulses >= SMOOTH_COUNT) {
				pulse_out(OUT_INC);
				pulses = 0;
			}
		} else {
			direction = DIR_UP;
			pulses = 0;
		}
	}
}
