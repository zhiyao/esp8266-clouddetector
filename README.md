# NodeMCU ESP8266 Remote Weather Station using WebSockets

This is inspired by https://www.hackster.io/rayburne/nodemcu-esp8266-remote-temperature-using-websockets-5956c4

## Prerequisite

### Hardware

- [MLX90614](https://www.banggood.com/MLX90614ESF-AAA-Non-contact-Human-Body-Infrared-IR-Temperature-Sensor-Module-For-Arduino-p-1100990.html?rmmds=myorder)
- [NodeMCU](https://www.banggood.com/NodeMcu-Lua-WIFI-Internet-Things-Development-Board-Based-ESP8266-CP2102-Wireless-Module-p-1097112.html?rmmds=myorder)
- [DHT22](https://www.banggood.com/Digital-DHT22-AM2302-Temperature-Humidity-Sensor-Module-For-Arduino-p-965081.html?rmmds=search)
- [BMP085](https://www.banggood.com/GY-68-300-1100hPa-BMP180-BOSCH-Temperature-Module-Atmospheric-Pressure-Sensor-Instead-Of-BMP085-p-1175978.html?rmmds=search)
- [PhotoResistor](https://www.banggood.com/10-PCS-5MM-Light-Dependent-Resistor-Photoresistor-GL5528-LDR-p-922619.html?rmmds=search)

### Software

- [SPIFFS file system installed](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.2.0/ESP8266FS-0.2.0.zip)
- Install ESP8266

## Installation

1. Open Arduino, Tools > board switch to be NodeMCU 1.0
2. Update the password and username of your wifi
3. Compile and Flash the NodeMCU
4. Tools > ESP8266 Sketch Data Upload to dump the data folder

## Usage

1. Use the serial monitor to figure out the Wifi Local IP address
2. Navigate on any browser where the device is connected to the same network to the IP address
