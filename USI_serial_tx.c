/*
 * USI_serial_tx.c
 *
 *      Author: Mark Osborne, BecomingMaker.com
 *      See accompanying post http://becomingmaker.com/
 */

#include "USI_serial_tx.h"
#include <avr/interrupt.h>

void sendStringP(PGM_P string)
{
	char c;
	while (0 != (c = pgm_read_byte(string)))
	{
		while (!usiserial_send_available())
		{
			// Wait for last send to complete
		}
		usiserial_send_byte(c);
		string++;
	}
}

void sendString(char* string)
{
	char c;
	while (0 != (c = *string))
	{
		while (!usiserial_send_available())
		{
			// Wait for last send to complete
		}
		usiserial_send_byte(c);
		string++;
	}
}

void usiserial_send_byte(uint8_t data)
{
    while (usiserial_send_get_state() != AVAILABLE)
    {
        // Spin until we finish sending previous packet
    };
    usiserial_send_set_state(FIRST);
    usiserial_set_tx_data(reverse_byte(data));

    // Configure Timer0
    TCCR0A = 2<<WGM00;                      // CTC mode
    TCCR0B = CLOCKSELECT;                   // Set prescaler to clk or clk /8
    GTCCR |= 1 << PSR0;                     // Reset prescaler
    OCR0A = FULL_BIT_TICKS;                 // Trigger every full bit width
    TCNT0 = 0;                              // Count up from 0

    // Configure USI to send high start bit and 7 bits of data
    USIDR = 0x00 |                            // Start bit (low)
        usiserial_get_tx_data() >> 1;         // followed by first 7 bits of serial data
    USICR  = (1<<USIOIE)|                     // Enable USI Counter OVF interrupt.
        (0<<USIWM1)|(1<<USIWM0)|              // Select three wire mode to ensure USI written to PB1
        (0<<USICS1)|(1<<USICS0)|(0<<USICLK);  // Select Timer0 Compare match as USI Clock source.
    DDRB  |= (1<<PB1);                        // Configure USI_DO as output.
    USISR = 1<<USIOIF |                       // Clear USI overflow interrupt flag
        (16 - 8);                             // and set USI counter to count 8 bits
}

uint8_t reverse_byte (uint8_t x) {
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;
}

// USI overflow interrupt indicates we have sent a buffer
ISR (USI_OVF_vect) {
	if (usiserial_send_get_state() == FIRST)
	{
		usiserial_send_set_state(SECOND);
		USIDR = usiserial_get_tx_data() << 7  // Send last 1 bit of data
				| 0x7F;                           // and stop bits (high)
		USISR = 1<<USIOIF |                   // Clear USI overflow interrupt flag
				(16 - (1 + (STOPBITS)));          // Set USI counter to send last data bit and stop bits
	}
	else
	{
		PORTB |= 1 << PB1;                    // Ensure output is high
		DDRB  |= (1<<PB1);                    // Configure USI_DO as output.
		USICR = 0;                            // Disable USI.
		USISR |= 1<<USIOIF;                   // clear interrupt flag

		usiserial_send_set_state(AVAILABLE);
	}
}

