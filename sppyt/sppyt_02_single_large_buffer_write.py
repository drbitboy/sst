#!/usr/bin/env python3

import sys
import serial

baudrate = None

def portsetup():
    sp = serial.Serial('/dev/ttyTHS0')
    sp.baudrate = baudrate
    sp.parity = 'E'
    sp.stopbits = 2
    return sp

def reader():
    sp = portsetup()
    buf = sp.read_until()
    print('{1}:  read {0} bytes'.format(len(buf), sys.argv[0]))


def writer():
    msg=(b's'*(baudrate-1))+b'\n'
    sp = portsetup()
    print('{1}:  wrote {0} bytes'.format(sp.write(msg), sys.argv[0]))

def usage():
    print("""
sppyt_02_4M.py:  run serial port test 02

- 4Mbytes written at 4Mbaud

Use two terminal windows, R and W; BASH shown

    Terminal R:  first

        % ./sppyt_02_4M.py reader

    Terminal W:  second, after reader above is started

        % ./sppyt_02_4M.py writer

""")

if "__main__" == __name__:
    baudrate =  list(map(int,[115200] + sys.argv[2:3])).pop()
    command = (['usage'] + sys.argv[1:2]).pop()
    func = dict(usage=usage,reader=reader,writer=writer)[command]
    print(dict(baudrate=baudrate,command=command))
    func()
