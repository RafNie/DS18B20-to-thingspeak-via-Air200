/*
 * config.h
 *
 *  Created on: 24 gru 2018
 *      Author: rafal
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/*
 * User configuration:
*/

// Sending interval in minutes max 65535
#define SENDING_INTERVAL 19

// GPRS config
#define APN "your apn"
#define APN_USER ""
#define APN_PWD ""

// Thingspeak config
#define API_KEY "your thingspeak key"
#define FIELD_TEMPERATURE_1 "1"
#define FIELD_VOLTAGE "2"

/*
 * Hardware configuration
 */

// Accumulator config

// Minimum value of LiIon accumulator voltage which is needed for communication via GPRS module
// Below MIN_VOLT_VAL the device sleeps until power voltage will grow
#define MIN_VOLT_VAL 3300 // mV

// CPU config
//#define F_CPU 7372800

#endif /* CONFIG_H_ */
