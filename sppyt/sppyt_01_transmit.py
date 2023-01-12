#!/usr/bin/env python3

import serial

message = b'Hello, World\n'
ser = serial.Serial('/dev/ttyTHS0')
ser.baudrate = 12500000
ser.write(message)

"""
Usage:

    % # => precede by by ./sppyt_01_receive.py in another terminal window
    % ./sppyt_01_transmit.py
"""
