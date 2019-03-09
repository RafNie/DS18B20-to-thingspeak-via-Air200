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

int16_t gTemperature = 0;

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

void init_single_Ucc_meas()
{
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
	ADMUX = (1<<MUX3) | (1<<MUX2); // set Ucc as reference and Vbg (1.1V) as input val.
	_delay_ms(2);
	ADCSRA |= (1<<ADSC); // Start conversion - first conversion may be not to accurate
	while (ADCSRA & (1<<ADSC)){} // wait for conversion
	ADCSRA |= (1<<ADSC); // Start conversion
}

uint16_t get_Ucc_val_when_ready()
{
	while (ADCSRA & (1<<ADSC)){} // wait for conversion
	uint16_t result = ADC;
	/*
	 * Theoretical factor is 1126400 = 1.1V*1024*1000
	 * Vbg (1.1V) can be inaccurate
	 * Calculate real factor:
	 * (real measured Ucc) / (Ucc measured by AVR ADC, calculated with 1126400) = correction factor
	 * example:
	 * 4.01 / 4.171 = 0,96140014385
	 * 1126400 * 0,96140014385 = 1082921
	 * Factor based on measured voltage is 1082921
	 */
	result = 1082921 / result;
	return result; // Ucc in millivolts
}

void convert_integer_part(int16_t temperature, char * string)
{
	   if (temperature & 0x8000) // check sign bit
	   {
		  *string = '-'; // it is necessary to manually print sign due to temperatures between -1 and 1
	      temperature = ~temperature + 1 ;
	   }
	   else
	   {
		   *string = '+';
	   }
	   itoa(temperature>>4, string+1, 10);
}

void convert_fractional_part(int16_t temperature, char * string)
{
	   if (temperature & 0x8000) // check sign bit
	   {
	      temperature = ~temperature + 1 ;
	   }
	   temperature = (temperature & 0x000F) * 625;
	   if (temperature <= 625)
	   {
		   *string = '0';
		   itoa((temperature & 0x000F) * 625, string+1, 10);
	   }
	   else
	   {
		   itoa((temperature & 0x000F) * 625, string, 10);
	   }
}

int main(void)
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); // set sleep mode: Power-down
	sei();

	DDRB  |= (1<<PB1); // setup serial output
	PORTB |= (1<<PB1);


	DDRB |= (1<<PB0); // set PB0 as output
	PORTB &= ~(1<<PB0);
	_delay_s(1);

	while(1)
	{
		init_single_Ucc_meas();
		// measure Ucc voltage without load (Air200 is off)
		if(MIN_VOLT_VAL > get_Ucc_val_when_ready())
		{
			//sleep and wait for charging battery by solar cell
			sleep_cpu_minutes(SENDING_INTERVAL);
			continue;
		}

		PORTB |= (1<<PB0); // turn on Air200

		// measure temperature
		ds18b20convert( &PORTB, &DDRB, &PINB, ( 1 << PB2 ), NULL );
		_delay_s(1);
		if (ds18b20read( &PORTB, &DDRB, &PINB, ( 1 << PB2 ), NULL, &gTemperature ))
		{
			_delay_s(10);
			continue; // if read fail - don't send nothing via http - try next time
		}

		// send AT command sequence
		_delay_s(1);
		sendStringP(cInit);
		_delay_s(5);
		if (PIN_EN)
		{
			sendStringP(cPIN);
		}
		_delay_s(35); // wait 35s for Air200 full initialization and network attachment
		sendStringP(cApn);
		_delay_s(3);
		sendStringP(cConnect);
		_delay_s(15);
		sendStringP(cSend);
		_delay_s(2);
		sendStringP(cHttpGetBegin);

		char buffer[10];
		convert_integer_part(gTemperature, buffer);
		sendString(buffer);
		sendString(".");
		convert_fractional_part(gTemperature, buffer);
		sendString(buffer);

		sendStringP(cVoltage);
		init_single_Ucc_meas(); // measure and report voltage under load (Air200 is on)
		uint16_t voltage = get_Ucc_val_when_ready();
		itoa(voltage/1000, buffer, 10);
		sendString(buffer);
		sendString(".");
		itoa(voltage%1000, buffer, 10);
		sendString(buffer);

		sendStringP(cHttpGetEnd);
		_delay_s(3);

		PORTB &= ~(1<<PB0); // turn off Air200
		sleep_cpu_minutes(SENDING_INTERVAL);
	}

	return 0;
}
