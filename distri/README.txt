EspLightNode precompiled package
================================
This is a pre-compiled version of the EspLightNode package.
Source code and documentation can be found at:
	https://github.com/Frans-Willem/EspLightNode

To flash this to your ESP8266, use esptool to flash all numbered .bin files to their appropriate location.
If needed, write blank.bin to 0x3F000 to reset EspLightNode's configuration. It might be needed to also re-flash the other firmware parts after this.

An example on linux would look like this.
Without configuration reset:
	esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash 0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin
With configuration reset:
	esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash 0x00000 firmware/0x00000.bin 0x3F000 firmware/blank.bin 0x40000 firmware/0x40000.bin

