/*
 *	w12_wirelessTransmitter.c
 *
 *	Created: 5/6/2013 12:07:26 PM
 *  Author: littleBits Electronics, Inc.
 *
 * Copyright 2014 littleBits Electronics
 *
 * This file is part of w12-wirelessTransmitter.
 *
 * w12-wirelessTransmitter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * w12-wirelessTransmitter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License at <http://www.gnu.org/licenses/> for more details.
 */ 


#include "3ch_analog_transmitter.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

volatile unsigned char counter_1_25ms = 0;
volatile unsigned char analogValue[C_CHANNEL_NUM];
volatile unsigned char outputValue[C_CHANNEL_NUM];
volatile BOOL Last_State_High[C_CHANNEL_NUM] = {FALSE, FALSE, FALSE};
volatile BOOL Last_State_Low[C_CHANNEL_NUM] = {FALSE, FALSE, FALSE};
volatile unsigned char analogValue_count = 0;
volatile BOOL new_analog_Data = FALSE;
volatile BOOL stTransmit = 0;

void Init_USART_DS_Mode(unsigned int ubrr)  // Initialize UART0 module in Double Speed Mode (U2X0 = 1)
{
	UBRR0H = (unsigned char)(ubrr>>8);		// Set baud rate
	UBRR0L = (unsigned char)ubrr;
	UCSR0A = (1 << U2X0);					// Double the USART Transmission Speed
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);			// Enable receiver and transmitter
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);		// Set frame format: 8data, 2stop bit
}

void Init_Timer1(void)
{
	TCNT1 = 0;
	OCR1A = (unsigned int)(F_CPU/800 - 1);	// f = 800 Hz
	OCR1B = (unsigned int)(F_CPU/800 - 1);	// f = 800 Hz Trigger source for ADC start conversion
	TCCR1B |= (1 << WGM12)|(1 << CS10);		// CTC Mode, Clock source = clkI/O/1 (No prescaling)
	TIMSK1 |= (1 << OCIE1A);				// Timer/Counter1, Output Compare A Match Interrupt Enable
	TIFR1 |= (1 << OCF1A);					// Timer/Counter1, Output Compare A Match Interrupt Reset
}

void Init_ADC(unsigned char prescaler)
{
	switch(prescaler)
	{
		case 2:
			ADCSRA = (1 << ADPS0);
			break;
		case 4:
			ADCSRA = (1 << ADPS1);
			break;
		case 8:
			ADCSRA = (1 << ADPS0)|(1 << ADPS1);
			break;
		case 16:
			ADCSRA = (1 << ADPS2);
			break;
		case 32:
			ADCSRA = (1 << ADPS2)|(1 << ADPS0);
			break;
		case 64:
			ADCSRA = (1 << ADPS2)|(1 << ADPS1);
			break;
		case 128:
			ADCSRA = (1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0);
			break;
	}
	
	ADMUX = (1 << REFS0);										// Voltage Reference is AVCC with external capacitor at AREF pin, Input channel ADC0
	ADCSRA = (1 << ADEN)|(1 << ADSC)|(1 << ADATE)|(1 << ADIE);	// ADC Enable, ADC Start Conversion, ADC Auto Trigger Enable, ADC Interrupt Enable
	ADCSRA |= (1 << ADIF);										// ADC Interrupt Flag Reset
	ADCSRB = (1 << ADTS2)|(1 << ADTS0);							// ADC Auto Trigger Source is Timer/Counter1 Compare Match B
}

ISR(TIMER1_COMPA_vect)
{	// Timer/Counter1 Output Compare A Match Interrupt Vector
	if (counter_1_25ms > 0)
	{
		counter_1_25ms--;
	}
	// Reset Timer/Counter1, Output Compare A and B Match Flags
	TIFR1 |= (1 << OCF1A)|(1 << OCF1B); 
}

ISR(ADC_vect)
{	// ADC Conversion Complete Interrupt Vector
	analogValue[analogValue_count] = (ADC >> 2);
	if(++analogValue_count >= 3)
	{
		analogValue_count = 0;
		// Voltage Reference is AVCC with external capacitor at AREF pin, Input channel ADC0
		ADMUX = (1 << REFS0); 
		for (unsigned char i = 0; i < C_CHANNEL_NUM; i++)
		{	// Save converted data
			outputValue[i] = analogValue[i];	
		}
		new_analog_Data = TRUE; // Process converted data
	}
	else
	{
		ADMUX++;
	}
}

void Send_data (unsigned char* buffer, unsigned char buffer_length)
{
	for (unsigned char i = 0; i < buffer_length; i++)
	{
		// Wait for empty transmit buffer
		while ( !( UCSR0A & (1<<UDRE0)) )
		;
		// Put data into buffer, sends the data
		UDR0 = buffer[i];
	}
}

void USART_Flush(void)
{
	unsigned char temp;
	while ( (UCSR0A & (1<<RXC0)) )
	{
		temp = UDR0;
	}
}

int main(void)
{
    Init_USART_DS_Mode(MYUBRR_DS_MODE);			// Initialize serial communications at 9600 bps
    Init_Timer1();								// Initialize Timer1 to generate interrupts at a rate of 800 Hz
    Init_ADC(ADC_PRESCALER);
	sei();										// Enable Interrupts
	USART_Flush();
	//_delay_ms(30);
	
	// clear the watchdog system reset flag
	MCUSR &= ~(WDRF << 1);
		
	// Enable the watchdog timer. We specify a max-out period of 60 mS
	wdt_enable(WDTO_60MS);
	
	while(1)
    {
        if(new_analog_Data) 
		{	// Process converted data
			
			new_analog_Data = FALSE;
			for (unsigned char i = 0; i < C_CHANNEL_NUM; i++)
			{	// Send 1 samples from each analog channel
				// Check all channels is more than input analog threshold
				if (!Last_State_Low[i])
				{	// Last state is HIGH
					if (outputValue[i] < C_ANALOG_THRESHOLD_ADC_LOW_MIN)
					{
						Last_State_Low[i] = TRUE;	// New state is LOW
						outputValue[i] = 0;
					}
				}
				else
				{	// Last state is LOW
					if (outputValue[i] > C_ANALOG_THRESHOLD_ADC_LOW_MAX)
						Last_State_Low[i] = 0;		// New state is HIGH
					else outputValue[i] = 0;		// Still LOW state
				}
			#ifdef C_LOGIC_HIGH_THRESHOLD
				// Logic HIGH threshold
				if (!Last_State_High[i])
				{	// Last state is LOW
					if (outputValue[0] > C_ANALOG_THRESHOLD_ADC_HIGH_MAX)
					{
						Last_State_High[i] = TRUE;	// New state is HIGH
						outputValue[i] = 255;
					}
				}
				else
				{	// Last state is HIGH
					if (outputValue[i] < C_ANALOG_THRESHOLD_ADC_HIGH_MIN)
						Last_State_High[i] = FALSE;	// New state is LOW
					else outputValue[i] = 255;		// Still HIGH state
				}
			#endif
			}			
							
			// Insert Start frame marker to first byte and clear LSB in other bytes
			outputValue[0] |= 0x01;
			outputValue[1] &= ~0x01;
			outputValue[2] &= ~0x01;
			if (!Last_State_Low[0] || !Last_State_Low[1] || !Last_State_Low[2])
			{	// At least one of the analog channels is more than the threshold - send data via radio
				Send_data(outputValue, sizeof(outputValue));
				counter_1_25ms = (TRANSMIT_TIMEOUT_MS / TIMER1_CLOCK_PERIOD_MS);	// Reset transmitter timeout
			}
			else
			{	// All analog channels less than threshold
				if (counter_1_25ms > 0)
				{
					Send_data(outputValue, sizeof(outputValue));
				}
			}
		}
		
	// Kick the dog AKA reset the watchdog timer, thereby avoiding an undesired system reset...
    wdt_reset();
	
	} // end  while(1)
} // end main()
