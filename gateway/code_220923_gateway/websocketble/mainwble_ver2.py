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
import socket


global data
#global pipe_data_ble
data = ""
ble_data = ""
pipe_data = ""
pipe_data_ble = ""

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

def websocket_write():
	
	global pipe_data
	global ble_data
	
	while True:
		try:
			with connect(PATHSOCKET) as websocket:
				#buf = "{\"name\":\"01-02-03\"," + pipe_data + "," + ble_data  + "}"
				#a = json.loads(buf)
				#websocket.send(a)
				if(pipe_data != ""):
					pipe_data = pipe_data.strip('\x00')
				buf = "{\"name\":\"" + CARIP+"\"," + pipe_data + "," + ble_data  + "}"
				#print("\nweb:" + buf)
				#if(pipe_data != "") and (ble_data != ""):#abc = json.loads(buf)
				websocket.send(buf)
				print("gui:",str(buf))
				#print(pipe_data.encode())
				#print(ble_data.encode())
			sleep(5)
		except:
			print("wifi error")
			sleep(5)




def pipe_recei():
	global pipe_data
	while True:
		with open(PATHPIPE,"r") as pipe:
			pipe_data = pipe.read()
			#print("recei pipe:" + pipe_data)
def ble():
	global ble_data
	global pipe_data_ble
	act = True
	while True:
		try:
			#print("act:" + str(act))
			if(act):
				p = btle.Peripheral(PATHBLE)
				s = p.getServiceByUUID("0000ffe0-0000-1000-8000-00805f9b34fb")
				c = s.getCharacteristics("0000ffe1-0000-1000-8000-00805f9b34fb")[0]
				act = False
				#c.write(bytes("n/a#","utf-8"))
			buffer = str(c.read(),'utf-8')
			#print('data ble:',buffer)
			
			if((buffer == 'n/a#') or (buffer == " ") or (buffer == "")):
				if(pipe_data_ble != ""):
					c.write(bytes((pipe_data_ble + '#'),"utf-8"))
					sleep(0.5)
					c.write(bytes("n/a#","utf-8"))
					pipe_data_ble = ""
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
		
		except Exception as e:
			try:
				print("disconnect")
				act = True
				time.sleep(1)
			except Exception as e:
				print("....error")          
		
def sendble():
	#print("@@@@@@@@@@@@@@@@@@@1")
	pipe_path_ble = '/root/wiringOP/code_220923_gateway/receiwebsocket/node'
	global pipe_data_ble
	while True:
		with open(pipe_path_ble,"r") as pipe:
			pipe_data_ble = pipe.read()
			print("recei pipe:" + pipe_data_ble)
    
if __name__ == "__main__":
	t1 = threading.Thread(target=websocket_write)
	t2 = threading.Thread(target=pipe_recei)
	t3 = threading.Thread(target=ble)
	t4 = threading.Thread(target=sendble)

	t1.start()
	t2.start()
	t3.start()
	t4.start()


