/*
 * config.h
 *
 *  Created on: 24 gru 2018
 *      Author: Rafal Niedzwiedzinski
 *
 *  Description:
 *  Simple ATtiny module for transfer DS18B20 temperature to thingspeak.com via Air200t
 *
 */

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

int gTemperature = 0;

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

void sleep_cpu_minutes(uint16_t minutes)
{
	if (0 != minutes)
	{
		//decrease by one minute, the total delay in the main loop lasts about a minute
		minutes -= 1;
	}

	cli();
	MCUSR = 0;
	WDTCR = (1<<WDCE) | (1<<WDE);
	WDTCR = (1<<WDIE) | WDTO_2S; // enable watchdog interrupt instead of reset
	sei();

	uint32_t max_intervals = 30 * minutes;
	while (max_intervals > 0)
	{
		sleep_mode();
		--max_intervals;
	}

	cli();
	WDTCR = 0; // disable watchdog
	sei();
}

int main(void)
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); // set sleep mode: Power-down
	sei();

	DDRB  |= (1<<PB1);  // setup serial output
	PORTB |= (1<<PB1);


	DDRB |= (1<<PB0); // set PB0 as output
	PORTB &= ~(1<<PB0);
	_delay_s(2);

	while(1)
	{
		PORTB |= (1<<PB0); // turn on Air200

		ds18b20convert( &PORTB, &DDRB, &PINB, ( 1 << PB2 ), NULL );
		_delay_s(1);
		if (ds18b20read( &PORTB, &DDRB, &PINB, ( 1 << PB2 ), NULL, &gTemperature ))
		{
			_delay_s(10);
			break; // break if read fail - don't send nothing via http
		}

		_delay_s(35); // wait for Air200 full initialization and network attachment
		sendStringP(cInit);
		_delay_s(3);
		sendStringP(cApn);
		_delay_s(5);
		sendStringP(cConnect);
		_delay_s(10);
		sendStringP(cSend);
		_delay_s(2);
		sendStringP(cHttpGetBegin);

		char buffer[10];
		itoa(gTemperature/16, buffer, 10);
		sendString(buffer);
		sendString(".");
		itoa(((gTemperature%16)*1000)/16, buffer, 10);
		sendString(buffer);

		sendStringP(cHttpGetEnd);
		_delay_s(10);

		PORTB &= ~(1<<PB0); // turn off Air200
		sleep_cpu_minutes(SENDING_INTERVAL);
	}

	return 0;
}
