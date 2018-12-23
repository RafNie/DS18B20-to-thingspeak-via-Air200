#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "prog_mem_strings.h"
#include "ds18b20/ds18b20.h"
#include "USI_serial_tx.h"

int temperature = 0;

ISR(WDT_vect)
{
	MCUSR = 0;
	wdt_reset();
}

void _delay_s(uint16_t sec)
{
	sec *= 100;
	for (uint16_t i=0; i<sec; i++)
	{
		_delay_ms(10);
	}
}

void sleep_cpu_minutes(const uint16_t minutes)
{
	PORTB &= ~(1<<PB0); // turn off Air200
	cli();
	MCUSR = 0;
	WDTCR = (1<<WDCE) | (1<<WDE);
	WDTCR = (1<<WDIE) | WDTO_2S; // enable watchdog interrupt instead of reset
	sei();
	const uint32_t max_intervals = 30 * minutes;
	uint32_t sleep_intervals = 0;
	while (sleep_intervals < max_intervals)
	{
		sleep_mode();
		sleep_intervals++;
	}

	cli();
	WDTCR = 0; // disable watchdog
	sei();

	PORTB |= (1<<PB0); // turn on Air200
}

int main(void)
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode: Power-down
	sei();

	PORTB |= 1 << PB1; // setup serial output
	DDRB  |= (1<<PB1);

	DDRB |= (1<<PB0); // set PB0 as output
	PORTB &= ~(1<<PB0); // write 0 to PB0
	_delay_s(2);
	PORTB |= (1<<PB0); // write 1 to PB0 - turn on Air200

	while(1)
	{
		ds18b20convert( &PORTB, &DDRB, &PINB, ( 1 << PB2 ), NULL );
		_delay_s(1);
		if (ds18b20read( &PORTB, &DDRB, &PINB, ( 1 << PB2 ), NULL, &temperature ))
		{
			_delay_s(10);
			break; // break if read fail - don't send nothing via http
		}

		_delay_s(35); // wait for Air200 full initialization
		sendStringP(cInit);
		_delay_s(2);
		sendStringP(cApn);
		_delay_s(5);
		sendStringP(cConnect);
		_delay_s(10);
		sendStringP(cSend);
		_delay_s(2);
		sendStringP(cHttpGetBegin);

		char buffer[10];
		itoa(temperature/16, buffer, 10);
		sendString(buffer);
		sendString(".");
		itoa(((temperature%16)*1000)/16, buffer, 10);
		sendString(buffer);

		sendStringP(cHttpGetEnd);
		_delay_s(10);

		sleep_cpu_minutes(SENDING_INTERVAL);
	}

	return 0;
}
