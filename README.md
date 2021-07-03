# GardenControl
A Arduino based relay controller for my garden using wifi and MQTT

## Hardware
- Basically the hardware mentioned in https://github.com/Jaycar-Electronics/WiFi-Relay-Controller
- A home wifi :)

## Software
- A mosquitto server (assumption: no authentication)

## Howto use
- Read https://github.com/Jaycar-Electronics/WiFi-Relay-Controller to get an idea of what needs to be don
- Copy the esp/data/settings.ini.tmlp to esp/data/settings.ini, adjust and upload as per documentation in docu/ directory 
- Deploy ESP and Uno code
- Check your MQTT server - you should see various channels pop up

