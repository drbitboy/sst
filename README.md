# sst
TTY stress test

### Manifest

#### Source code and makefile
* raw_settings.h
* sst.c
* sst.h
* stty_info.h
* Makefile

#### TTY settings

    COLUMNS=1 stty -F /dev/tty... -a | LC_ALL=C sort

* RAW - determined by experiment
  * raw.sst.settings.txt
  * backup-usb-settings.tar
  * usb0.stty.txt

* Initial default immediately USB serial port plugged in
  * init.usb.tty.txt

* After stty -F /dev/tty... sane
  * sane.usb0.tty.txt

#### This file
* README.md
