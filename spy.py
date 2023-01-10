#!/usr/bin/env python3

import sys
try   : import serial
except: pass

opts = dict(baudrate=12500000
           ,port='/dev/ttyTHS0'
           ,stopbits=2
           ,parity='E'
           ,timeout=10.0
           )

def reader():
    ser = serial.Serial(**opts)
    ct = 0
    for buf in ser.iread_until():
        if not ct: print((buf,))
        if ct > 940093: print(ct) ; ct -= 940094
    print(dict(close=ser.close()))

def writer():
    ser = serial.Serial(**opts)
    buf = bytes(list(range(33,127))+[10])
    mL = -len(buf) - 1
    total_count = opts['baudrate']
    log_count = total_count - 500000
    while total_count > 0:
        for mi in range(-1,mL,-1):
            incr_count = ser.write(buf[mi:])
            if incr_count > 0:
                total_count -= incr_count
                if total_count <= log_count:
                    print(total_count)
                    log_count -= 500000
    print(dict(close=ser.close()))

if "__main__" ==__name__ and sys.argv[1:]:

    for arg in sys.argv[1:]:
        if arg in set('reader writer'.split()):
            func = dict(reader=reader,writer=writer)[arg]
            continue
        key,sval = (arg.split('=') + [True])[:2]
        try   : val = eval(sval)
        except: val = sval
        opts[key] = val
        
    func()
