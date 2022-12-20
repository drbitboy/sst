# sst
TTY stress test - write test data to a serial port

## Usage

    [make [clean ]all]
    ./sst[ --option[=...] [--option... [...]]]

See run_sst.sh for typical executions

## Current experience

* Used with loopback in place, so any characters written can be read and checked
* Laptop with USB-dongle serial port
  * Works _at some baudrates_ on USB-dongle serial port (--tty=/dev/ttyUSB0) on a laptop under recent Ubuntu distribution
    * Starts missing characters at higher baud rates (115.2kbaud or higher)
* NVIDIA Jetson platform with Tegra High-Speed serial port
  * See sub-directory Jetson/README.md
   * Works _at some baudrates_ on Tegra High-Speed serial port (--tty=/dev/ttyTHS0)
     * At lower baudrates (at least up to 115.2k, and probably higher)
       * All characters that are written appear are read
     * At 4Mbaud
       * Starts missing characters (order 1E-5)
     * At 8Mbaud, 12Mbaud and 12.5Mbaud
       * **_Entire system crashes_** and reboots
         * _At first_, a soft reset (reboot) recovers functionality
         * Eventually, system is unable to write and/or read data from THS0,
           * _At any baudrate_
           * A hard reset is required to restore proper functionality

### Command-line Options
* --tty=/dev/tty...
  * Write data to typical TTY device in filesystem
  * E.g. --tty=/dev/ttyTHS0 or --tty=/dev/ttyUSB0
* --speed=BAUDRATE
  * Set TTY speed (baudrate)
  * See file stty_info.h for pre-programmed speeds
    * e.g. --speed=12.5M, --speed=12500000, --speed=115200
  * Default is to use the current TTY speed
* --baud=BAUDRATE
  * Synonym for --speed=BAUDRATE
* --send-count=12500000
  * How many characters to send
* --open-non-blocking
  * Open the TTY non-blocking
    * N.B. default is to open for blocking
* --fork-reader
  * Fork a process to read the data written
    * N.B. default is to not fork a reader
* --do-raw-config
  * Perform configuration of the TTY to pass raw data
    * N.B. default is to not perform raw configuration
      * The [stty] utility can be used to pre-configure most options
* --non-standard-tty=PATH
  * Write data to non-typical TTY device (or file)
  * E.g. --non-standard-tty=sst_test_data.txt
* --raw-config-debug
  * Print details of the configuration of the TTY to pass raw data
* --raw-config-debug-speed
  * Print details the configuration of the TTY speed
* --debug
  * Log several of the steps
* --dump-raw-settings=PATH
  * Dump raw setting string from raw_settings.h to file specified by PATH
  * Defaults to STDOUT if [=PATH] is not supplierd
* --dump-to-send[=PATH]
  * Dump [to_send] character array from ssth.h into file specified by PATH
  * Defaults to STDOUT if [=PATH] is not supplierd

### Manifest

#### Script to run sst application; to be edited as needed
* run_sst.sh

#### Script to configure user permissions, and stop getty, on Tegra serial port /dev/ttyTHS0
* sudo_ttyT_config.sh

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
