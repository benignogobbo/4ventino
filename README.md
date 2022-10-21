# 4ventino

## 20190711

## hardware
- On Arduino IDE, Select "Tools"->"Borad"->"Boards Manager..."
  Install MightlyCore by MCUdude
     on Linux, in ~/.arduino15/packages/MightyCore/hardware/avr/2.0.2/libraries/
     rename Ethernet Ethernet.nouse

## libraries
- Download and add the following libs:
  Ethernet  from https://github.com/PaulStoffregen/Ethernet
  TimeLib   from https://github.com/PaulStoffregen/Time
  DS1307RTC from https://github.com/PaulStoffregen/DS1307RTC
  DS1302RTC from https://playground.arduino.cc/Main/DS1302RTC
  ITEADLIB  from https://github.com/itead/ITEADLIB_Arduino_Nextion
     here edit NexConfig.h and define nexSerial as Serial1

## tricks
- get the avrdude command executed by IDE, select "File"->"Preferences"
  Tick "upload" box in "ShowVerbose output during:"

## avrdude
- Windows (example):
C:\Program Files (x86\Arduino\hardware\tools\avr/bin/avrdude
-CC:\Users\benigno\AppData\Local\Arduino15\packages\MightyCore\hardware\avr\2.0.2/avrdude.conf
-v
-patmega1284p
-carduino
-PCOM29
-b115200
-D
-Uflash:w:C:\Users\benigno\AppData\Local\Temp\arduino_build_375178/4ventino-0.1-20190627.ino.hex:i

- Linux (example):
/home/benigno/arduino-1.8.9/hardware/tools/avr/bin/avrdude
-C/home/benigno/.arduino15/packages/MightyCore/hardware/avr/2.0.2/avrdude.conf
-v
-patmega1284p
-carduino
-P/dev/ttyUSB0
-b115200
-D
-Uflash:w:/tmp/arduino_build_342726/4ventino-0.1-20190627.ino.hex:i
