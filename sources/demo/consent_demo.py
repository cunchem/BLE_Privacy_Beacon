import threading
import time
import serial
from datetime import datetime
import subprocess


#c = threading.Condition()
flag = 0      #shared between Thread_A and Thread_B
val = 20
ser = serial.Serial('/dev/ttyUSB1', 115200)

trackingdata = []
whitelist = []
consents = []



class Serial_thread(threading.Thread):
    def __init__(self, name):
        threading.Thread.__init__(self)
        self.name = name

    def run(self):
        global whitelist
        global consents
        #global val     #made global here
        
        while True:
            line = ser.readline()
            #print line
            mac = line.split('{')[1].split('}')[0]
            consent = line .split('::')[1]
            #print mac
            if (mac not in whitelist):
                whitelist += mac
                consents += consent

class Tracking_thread(threading.Thread):
    def __init__(self, name):
        threading.Thread.__init__(self)
        self.name = name

    def run(self):
        global trackingdata
        #global val    #made global here
        bashCommand = "sudo hcitool -i hci0 lescan"
        print bashCommand
        process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE )
        for l in iter(lambda: process.stdout.readline(), ''):
            #print  l
            if "::Consent::" not in l: 
                continue
            t = str(datetime.now())
            mac = l.split(' ')[0]
            #print mac, t, "\n",
            if (mac in whitelist):
                print "storing data item", mac, t, "(have consent)"
            else :
                print "discarding data item", mac, t, "(no consent)"



a = Serial_thread("Serial Thread")
b = Tracking_thread("Tracking Thread")

b.start()
a.start()

a.join()
b.join()
