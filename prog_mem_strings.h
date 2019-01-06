/*
 * prog_mem_strings.h
 *
 *  Created on: 23 gru 2018
 *      Author: rafal
 */

#ifndef PROG_MEM_STRINGS_H_
#define PROG_MEM_STRINGS_H_

#include <avr/pgmspace.h>
#include "config.h"

const char cInit[] PROGMEM = "AT+CFUN=1\r";
const char cApn[] PROGMEM = "AT+CSTT=\""APN"\",\""APN_USER"\",\""APN_PWD"\"\r";
const char cConnect[] PROGMEM = "AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r";
const char cSend[] PROGMEM = "AT+CIPSEND\r";
const char cHttpGetBegin[] PROGMEM = "GET http://184.106.153.149/update?api_key="API_KEY"&field"FIELD_TEMPERATURE_1"=";
const char cVoltage[] PROGMEM = "&field"FIELD_VOLTAGE"=";
const char cHttpGetEnd[] PROGMEM = "\r\n\r\n\x1A";

#endif /* PROG_MEM_STRINGS_H_ */
