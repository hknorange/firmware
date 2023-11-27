'''
recei data
reconnect websockets
write data in file
"ws://146.190.155.39:8080"
'''

'''
code act
reconnect websocketclient websocket
listen data from client another
when recei data
    check in file ipcar.txt
if header == ipcar.txt
    send data to pipe ble, to ble send data to node
'''



import websocket
import _thread
import time
import rel
import json
import os

filo_path = '/root/wiringOP/code_220923_gateway/receiwebsocket/node'
#{'config': '34-1234', 'setup': {'volume': '400.50', 'hight': 'abc', 'weight': '12345'}}
def send_fifo(data):
    with open(filo_path,'w') as pipe:
        pipe.write(data)

def check(data):
    print('data:',type(data))
    print(data['volume'])
    print(data['hight'])
    print(data['weight'])
    data_send = data['volume'] + ',' + data['hight'] + ',' + data['weight']
    print(data_send)
    send_fifo(data_send)
    
    #pass

def on_message(ws, message):
    print(message)
    json_obj = json.loads(message)
    if "config" in json_obj:
        #print(json_obj["setup"])
        check(json_obj["setup"])
        
    else:
        pass

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print("### closed ###")

def on_open(ws):
    print("Opened connection")

if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://146.190.155.39:8080",
                              on_open=on_open,
                              on_message=on_message,
                              on_error=on_error,
                              on_close=on_close)

    ws.run_forever(dispatcher=rel, reconnect=5)  # Set dispatcher to automatic reconnection, 5 second reconnect delay if connection closed unexpectedly
    rel.signal(2, rel.abort)  # Keyboard Interrupt
    rel.dispatch()
