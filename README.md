# riban Raspberry Pi Interface
C++ bindings and wrapper to many Raspberry Pi interfaces

Implemented features:
* Raspberry Pi model (string and number)
* Configure GPI pins for each function type
* GPI input with pull-up / pull down
* GPI output
* Millisecond counter (32-bit / 49 day)

Not (yet) implemented:
* UART
* SPI
* Interrupts
* PCM / I2S Audio
* PWM
* SPI
* I2C

There are other libraries available for Raspberry Pi interfacing, including [wiringPi](http://wiringpi.com/) and [bcm2835](http://www.airspayce.com/mikem/bcm2835/). ribanrpiinterface aims to provide a simple C++ wrapper to allow easy access to commonly required features and to be scaleable, allowing disabling of features if the target platform has resource constraints.
