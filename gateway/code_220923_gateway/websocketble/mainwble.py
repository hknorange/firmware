'''
	websocket client
	ble client
	send data websocket server
	recei ble
'''
from threading import Thread
import threading
from time import sleep
import json
import sys
import asyncio
from websockets.sync.client import connect
import bluepy.btle as btle


global data

data = ""
ble_data = ""
pipe_data = ""


PATHPIPE = "/tmp/myfifo"
PATHSOCKET = ""
PATHBLE = ""

CARIP = ""

with open('/root/wiringOP/code_220923_gateway/ip/ipwebsocket.txt','r') as file:
    PATHSOCKET = file.read()
    print(PATHSOCKET)
with open('/root/wiringOP/code_220923_gateway/ip/ipble.txt','r') as file:
    PATHBLE = file.read()
    print(PATHBLE)
with open('/root/wiringOP/code_220923_gateway/ip/ipcar.txt','r') as file:
    CARIP = file.read()
    print(CARIP)

#pipe_data = ""
#PATHSOCKET = "ws://146.190.155.39:8080"
#PATHBLE = ""
#p = btle.Peripheral("04:A3:16:A1:4E:A9")
PATHBLE = PATHBLE.strip('\n')
#print(PATHBLE.encode())
PATHSOCKET = PATHSOCKET.strip('\n')
#print(PATHSOCKET.encode())
CARIP = CARIP.strip('\n')
#print(CARIP.encode())

p = btle.Peripheral(PATHBLE)
s = p.getServiceByUUID("0000ffe0-0000-1000-8000-00805f9b34fb")
c = s.getCharacteristics("0000ffe1-0000-1000-8000-00805f9b34fb")[0]

def websocket_write():
	global pipe_data
	global ble_data
	while True:
		with connect(PATHSOCKET) as websocket:
			#buf = "{\"name\":\"01-02-03\"," + pipe_data + "," + ble_data  + "}"
			#a = json.loads(buf)
			#websocket.send(a)
			if(pipe_data != ""):
				pipe_data = pipe_data.strip('\x00')
			buf = "{\"name\":\"" + CARIP+"\"," + pipe_data + "," + ble_data  + "}"
			#print("\nweb:" + buf)
			#if(pipe_data != "") and (ble_data != ""):
            #abc = json.loads(buf)
			websocket.send(buf)
			print(pipe_data.encode())
			print(ble_data.encode())
		sleep(5)

def pipe_recei():
	global pipe_data
	while True:
		with open(PATHPIPE,"r") as pipe:
			pipe_data = pipe.read()
			#print("recei pipe:" + pipe_data)
def ble():
	global ble_data
	while True:
		buffer = str(c.read(),'utf-8')
		if((buffer == 'n/a#') or (buffer == " ") or (buffer == "")):
			sleep(0.01)
		else:
			#ble_data = buffer
			print("\nble data:")
			print(buffer.encode())
			sleep(0.01)
			c.write(bytes("n/a#","utf-8"))
			sleep(0.1)
			data_split = buffer.split(",")
			if(len(data_split)>1):
				data_split[1] = data_split[1].strip('\x00')
			#print('\n')
				ble_data = "\"sensor\":{\"fuel\":" + data_split[0] + "," + "\"weight\":" + data_split[1] + "}"
			#print(ble_data)
			

    
if __name__ == "__main__":
	t1 = threading.Thread(target=websocket_write)
	t2 = threading.Thread(target=pipe_recei)
	t3 = threading.Thread(target=ble)
	t1.start()
	t2.start()
	t3.start()

