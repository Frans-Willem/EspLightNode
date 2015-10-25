EspLightNode compilation guide
==============================
Prerequisites
-------------
To compile EspLightNode, you need:
- G++ and friends for XTensa106-elf architecture
- newlib compiled with -mlongcalls
- Libstdc++ compiled with -mlongcalls
- GNU tools (grep, awk, make, print)
- esptool.py (https://github.com/themadinventor/esptool)

Recommended setup
-----------------
Most testing and compilation was done on ArchLinux, using the XTensa packages from:
	https://github.com/Frans-Willem/arch-esp8266-packages/
And the esptool.py package from:
	https://aur.archlinux.org/packages/esptool-git/

Compiling
---------
Just typing 'make' is usually enough.
'make flash' will compile and immediately try to flash to the device
'make clearconfig' will compile, flash, and erase any previous configuration on the device.
'make release' will make a release .zip file in the release directory.
