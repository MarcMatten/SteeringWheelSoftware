import serial
import struct
import serial.tools.list_ports as ListPorts
import time

PortList = ListPorts.comports()

for i in PortList:
    print(i.device)
    ser = serial.Serial(i.device, 9600, timeout=1, writeTimeout=0)
    time.sleep(0.5)
    ser.write(struct.pack('<f', 0.25))

    msg = ser.readline()
    if msg:
        print(struct.unpack('<f', msg)[0])

    
    ser.close()


##while True:
##	msg = er.readline()
##	if msg:print(msg)
