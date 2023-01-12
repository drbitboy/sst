# Serial Port Python Testing
Write test data to a serial port using Python PySerial module

* Sub-directory sppyt/ of this repo

#### Serial-port Python Tests
* sppyt_00_looper.py

* sppyt_01_receive.py,sppyt_01_transmit.py
  * Duplicates "Hello World" test in **Serial Ports** section of VPX-NSV-201_SpaceVPXBringupGuide-011222-1914-28.pdf

* sppyt_02_single_large_buffer_write.py
  * Extends "Hello World" test (above) to write data continously for approximately 12s.

#### Script to configure user permissions, and stop getty, on Tegra serial port /dev/ttyTHS0
* ../sudo_ttyT_config.sh
  * Usage:
    . ../sudo_ttyT_config.sh

    => enter account password for SUDO

#### This file
* README.md
