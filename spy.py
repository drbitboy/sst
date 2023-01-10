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
    total_count = 0
    for buf in ser.iread_until():
        total_count += len(buf)
        if not ct:
            print((buf,))
        elif ct > 94093: print(ct) ; ct -= 94094
        ct += 1
    opts.update(dict(close=ser.close(),total_count=total_count))
    print(opts)

def writer():
    ser = serial.Serial(**opts)
    buf = bytes(list(range(33,127))+[10])
    mL = -len(buf) - 1
    baudrate = total_count = opts['baudrate']
    dlog_count = total_count // 25
    log_count = total_count - dlog_count
    while total_count > 0:
        for mi in range(-1,mL,-1):
            incr_count = ser.write(buf[mi:])
            if incr_count > 0:
                total_count -= incr_count
                if total_count <= log_count:
                    print(total_count)
                    log_count -= dlog_count
    opts.update(dict(close=ser.close(),total_count=baudrate-total_count))
    print(opts)

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
