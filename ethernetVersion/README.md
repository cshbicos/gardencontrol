# GardenControl

An Arduino based relay controller for my garden using an Uno with an Ethernet shield

## Hardware

- Arduino Uno
- An ethernet shield
- A relay (e.g. the XC4418 from Jaycar)
- A moisture sensor kit

## Software

- A mosquitto server (assumption: no authentication)
- The Arduino PublishSubscribe Library
- The Arduino Ethernet Library
- The Arduino EEPROM Library

## Howto use

- Set up the wiring for the relay as per [this github page](https://github.com/Jaycar-Electronics/WiFi-Relay-Controller) on digital pins 2 through 9
- Set up the wiring for the moisture controller as per [this tutorial](https://projecthub.arduino.cc/Aswinth/soil-moisture-sensor-with-arduino-91c818) on analogue pin A0
- Connect the Arduino Ethernet shield to a network with DHCP
- Port 80 should bring up a webserver in which MQTT server details can be maintained
- Once connected (there should be a green message stating such) - various channels should appear on the MQTT server.
  - /{topic}/relay/{X} should contain "ON" or "OFF" and can be manipulated by MQTT clients to switch the relay
  - /{topic}/moisture/0 contains the moisture sensor's analogue read-outs

