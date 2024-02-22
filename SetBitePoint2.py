import serial
import time
import struct

ser = serial.Serial('COM11', 9600, timeout=0.1, writeTimeout=0)

while True:
    msg = ser.readline()
    if msg:
        if len(msg) == 1:
            data = struct.unpack('<b', msg)[0]
        elif len(msg) == 4:
            data = struct.unpack('<f', msg)[0]
        else:
            data = None
        print(data)
