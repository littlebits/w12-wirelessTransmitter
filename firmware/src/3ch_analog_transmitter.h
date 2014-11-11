/*
 * RFD21733 RFDP8  3-Channel Triple-Mode Wireless Encoder
 * Copyright (c) RF Digital Corporation
 * 1601 Pacific Coast Highway, Suite 290
 * Hermosa Beach, CA, 90254
 * www.rfdigital.com
 * Version: 2.0
 * Date: May 2nd, 2013
 */

#define SetBit(a,b)    ((a) |= (1<<(b)))
#define ClrBit(a,b)    ((a) &= ~(1<<(b)))
#define ToggleBit(a,b) ((a) ^= (1<<(b)))
#define TestBit(a,b)   ((a) & (1<<(b)))

typedef enum{FALSE=0,TRUE} BOOL;
	
#define F_CPU							1000000 // Clock Speed in Hz
#define BAUD							9600
#define MYUBRR_DS_MODE					F_CPU/8/BAUD-1
#define ADC_PRESCALER					16 // ADC Clock = 1000000 / 16 = 62500 Hz
#define TIMER1_CLOCK_PERIOD_MS			1.25
#define TRANSMIT_TIMEOUT_MS				100	// Timeout in milliseconds of shutdown the transmitter if all three channels are below the threshold

//#define C_LOGIC_HIGH_THRESHOLD  // Uncomment to enable Input logic HIGH threshold
#define C_DATA_LEN                         12  // Data frame length
#define C_CHANNEL_NUM                      3   // Total ADC/DAC Channels quantity
#define C_ANALOG_THRESHOLD_LOW             250UL // Analog input LOW threshold in mV
#define C_ANALOG_THRESHOLD_HIGH            4500UL // Analog input HIGH threshold in mV
#define C_ANALOG_THRESHOLD_HYSTERESIS_LOW  25UL  // Analog input LOW hysteresis in mV
#define C_ANALOG_THRESHOLD_HYSTERESIS_HIGH 25UL  // Analog input HIGH hysteresis in mV
#define C_ANALOG_THRESHOLD_ADC_LOW_MIN     ((C_ANALOG_THRESHOLD_LOW - C_ANALOG_THRESHOLD_HYSTERESIS_LOW)*256)/5000
#define C_ANALOG_THRESHOLD_ADC_LOW_MAX     ((C_ANALOG_THRESHOLD_LOW + C_ANALOG_THRESHOLD_HYSTERESIS_LOW)*256)/5000
#define C_ANALOG_THRESHOLD_ADC_HIGH_MIN    ((C_ANALOG_THRESHOLD_HIGH - C_ANALOG_THRESHOLD_HYSTERESIS_HIGH)*256)/5000
#define C_ANALOG_THRESHOLD_ADC_HIGH_MAX    ((C_ANALOG_THRESHOLD_HIGH + C_ANALOG_THRESHOLD_HYSTERESIS_HIGH)*256)/5000

#define ANALOG_IN_PIN_1   0 // Analog In 1
#define ANALOG_IN_PIN_2   1 // Analog In 2
#define ANALOG_IN_PIN_3   2 // Analog In 3

