import win32api
import time
import math
import serial
import ctypes
import struct

def GetClipCursor():
    _GetClipCursor = ctypes.windll.User32.GetClipCursor
    lprect = ctypes.create_string_buffer(ctypes.sizeof(ctypes.c_long)*4)
    _GetClipCursor(lprect)
    return struct.unpack("LLLL",lprect.raw)
x_pc = 0
y_pc = 0

#Define comm port
com_port = 4
#auto set screen resolution
box = GetClipCursor()
x_max = box[2]
y_max = box[3]
print str(x_max) +" " + str(y_max)

while (1):
    #Read x and y from screen
    ser = serial.Serial(com_port -1)    # open COMM port
    #print ser.portstr                   # check which port was really used
    ser.write("x")                      # write to get x
    x = ser.read(3)
    x1 = hex(ord(x[1]))
    x0 = hex(ord(x[2]))
    x = x1[2:] + x0[2:]
    ser.write("y")                      # write to get y
    y = ser.read(3)
    y1 = hex(ord(y[1]))
    y0 = hex(ord(y[2]))
    y = y1[2:] + y0[2:] 
    ser.close()                         # close port
    if ((int(x,16) > 10) & (int(y,16) > 30)): 
        #Map adc values to mouse positions
        x_pc = x_max - (long((x_max/600)*int(x,16)))
        y_pc = long((y_max/600)*long(y,16)) - 140
        #print str(x_pc) + " " + str(y_pc)
        print "Cursor : " + str(int(x_pc)) + " " + str(int(y_pc))
        win32api.SetCursorPos((int(x_pc),int(y_pc)))
        #debug
        print "y " + str(int(y,16))
        print "x " + str(int(x, 16))
time.sleep(.001)
