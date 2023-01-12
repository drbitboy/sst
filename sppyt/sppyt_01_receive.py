#!/usr/bin/env python3

import serial

ser = serial.Serial('/dev/ttyTHS0')
ser.baudrate = 12500000
print(ser.read_until())

"""
Usage:

    % ./sppyt_01_receive.py
    % # => followed by ./sppyt_01_transmit.py in another terminal window
"""
