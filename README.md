# DS18B20 thingspeak GPRS sender 

Project of simple controller designed for sending data from DS18B20 temperature sensor to thingspeak service using Air200t GPRS module.
The ATTiny obtains temperature data from DS18B20 sensor, prepare TCP session with thingspeak server via serial at the GPRS module and sends temperature value. All with use AT commands. Process is repeated in infinite loop.

The device can be powered from a LiIon battery charged with a solar cell.


## Hardware

The device is based on ATtiny45. The design is as simple as possible. An external oscillator was used to maintain stable serial communication in wide range of environment temperatures.

The microcontroller communicates with the GPRS module using AT commands. It was tested with Air200t module. I suppose that it should work also with other gprs modules.

Power source for a gprs module is switched by the NMOS transistor. It allow to limit of wasting energy between sending periods. 

![device schema](schema.png)

There are possible two types of powering:
1. Powering directly from 5V source.
2. Powering from LiIon accumulator charged with a solar cell. I used 6V/30mA solar cell for powering. Solar cell is working as current source after reaching of maximum current. Thanks of that there is not necessary to use additional charging controller. It is required to use protected LiIon battery to limit maximum charging voltage value.

The Air200t module has the voltage regulator, so it is impossible to power it directly from the LiIon accumulator using Ucc pin. It is necessary to solder an additional power cable to the filtering capacitor, as shown in the picture. The capacitor are connected to output form the voltage regulator. Accepted voltage range here is 3.3V-4.2V. 

<img src="powering.jpg" width="400">


Universal SO16 to DIP16 adapter PCB was used for mounting all components and connectors. 

<img src="top.jpg" width="200">

<img src="bottom.jpg" width="200">


An example of a complete system

<img src="system.jpg" width="400">


## Software

Project is developed under Eclipse with the AVR plugin. Project files are attached.

Program is simple loop. First a temperature is fetched from DS18B20 and next AT command sequence is send to Air200t gprs module. After that the module is switched off till the next sending time.
During this time interval the AVR cpu waits in energy saving mode PWR_DOWN and GPRS module is off. Sensor DS18B20 is always connected to power supply, but standby power consumption is declared only on 1uA. 


#### Settings

There is need to set thingspeak channel and gprs connection parameters (APN) in the __config.h__ file before project compilation. Parameters are commented in the file.

Serial baud rate is defined on 19200 bps. Change baud rate in GPRS module before use by AT command:
`AT+IPR=19200`

It is possible to send a PIN code to the SIM card. Configuration is possible in the __config.h__ file.


#### AVR Fuses

efuse: FF
hfuse: DF
lfuse: ED


#### Used external projects

A [avr-ds18b20 library](https://github.com/Jacajack/avr-ds18b20) was used for sensor communication.

Serial communication using ATtiny USI module is based on [USI Serial UART Send on ATtiny](http://becomingmaker.com/usi-serial-send-attiny/) article.


## License

This project is licensed under the MIT.

