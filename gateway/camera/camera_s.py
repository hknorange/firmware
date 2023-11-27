#camera tx

import socket,cv2, pickle,struct
import pyshine as ps # pip install pyshine
import imutils # pip install imutils
from time import sleep
import os
import sys
from datetime import datetime
from flask import Flask, Response, render_template
import threading
app = Flask(__name__)

success = ""
frame = ""

# read file txt get name config
def get_name_sd():
    output = os.popen('df -h | awk \'$6 ~ /\/media\// {print $6}\'').read() # command mounted
    output = output.strip('\n')
    return output

def get_port_device():
    with open('/root/wiringOP/code_220923_gateway/portusbcamera/port_usb_2_0_1.txt','r') as file:
        output = file.read()
    return output
def get_ip_servertcp():
    with open('/root/wiringOP/code_220923_gateway/ip/iptcpserver.txt','r') as file:
        output = file.read()
        if('\n' in output):
            output = output.strip('\n')
    return output
def get_ip_car():
    with open('/root/wiringOP/code_220923_gateway/ip/ipcar.txt','r') as file:
        output = file.read()
    return output
usb_port = get_port_device()
name_card = get_name_sd() # get mounted of sd card
host_ip = get_ip_servertcp()
car_ip = get_ip_car()
print("name card:",name_card)
print("usb port:",usb_port)
print("host ip:",host_ip)
print("car ip:",car_ip)
while(name_card == ""):
    name_card = get_name_sd()
    sleep(5)
    if(name_card != ""):
        break
cap = cv2.VideoCapture(usb_port)
cap.set(cv2.CAP_PROP_FPS,10)
size_frame = (640,480)

if(cap.isOpened()):
    now = datetime.now()
    date_time = now.strftime('back_date_%d_%m_%Y time_%H_%M_%S')
    path_save = os.path.join(name_card,date_time)
    #os.mkdir(path_save)
    result = cv2.VideoWriter(path_save + '.avi',cv2.VideoWriter_fourcc(*'XVID'),10,size_frame)
def generate_frames():
    while True:
        if not success:
            break
        else:
            web_frame = frame
            ret, buffer = cv2.imencode('.jpg', web_frame)
            frame1 = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame1 + b'\r\n')

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/video')
def video():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')
def task1():
    app.run(host='0.0.0.0', port=8881)
def task2():
    global success
    global frame
    global display_frame
    client_socket = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    #host_ip = '103.186.146.91'
    port = 9999
    client_socket.connect((host_ip,port))
    while True:
        success, frame = cap.read()
        now = datetime.now()
        date_time = now.strftime('%d-%m-%Y %H:%M:%S')
        cv2.putText(frame,date_time,(5,20),cv2.FONT_HERSHEY_PLAIN,1.5,(40,0,0),1,cv2.LINE_AA)
        #display_frame = frame
        result.write(frame)
        tframe = imutils.resize(frame,width=380)
        message_dict = {
            'frame': tframe,
            'license_car': car_ip,
            'position_camera': '3'
        }
        a = pickle.dumps(message_dict)
        message = struct.pack("Q",len(a))+a
        client_socket.sendall(message)
        #cv2.imshow("abc",frame)
        #print('frame',frame)
        '''
        out.write(frame) #ghi file mp4
        frame_tcp = frame # assign frame camera, create variable another
        frame = imutils.resize(frame_tcp,width=640,height=480)        #resize frame tcp 
        a = pickle.dumps(frame_tcp) # binary
        message = struct.pack("Q",len(a)) + a
        client_socket.sendall(message)
        '''
        '''
                    frame = imutils.resize(frame,width=380)
				message_dict = {
					'frame': frame,
					'license_car': '59A1-12345',
					'position_camera': '1'
				}
				a = pickle.dumps(message_dict)
				message = struct.pack("Q",len(a))+a
				client_socket.sendall(message)
				#cv2.imshow(f"TO: {host_ip}",frame)
				key = cv2.waitKey(1) & 0xFF
               '''        
        if (cv2.waitKey(1) == ord('q')):
            cv2.destroyAllWindows()
            break; 
        #print(frame)
'''def task3():
    connected = True
    client_socket = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    host_ip = '103.186.146.91'
    port = 9999
    client_socket.connect((host_ip,port))
    camera_ip,camera_port = client_socket.getsockname()
    print('ip camera:',camera_ip)
    print('port camera:',(camera_port))
    #select_port = 'camera tx:' + str(camera_port)
    #client_socket.sendall(select_port.encode('utf-8'))    
    while True:
        
        #frame_tcp = imutils.resize(frame,width=640,height=480)
        print(frame)
        tcpframe = imutils.resize(frame,width=380)
        message_dict = {
					'frame': tcpframe,
					'license_car': '59A1-12345',
					'position_camera': '1'
		}
        a = pickle.dumps(message_dict)
        message = struct.pack("Q",len(a)) + a
        client_socket.sendall(message)
        
'''        
    
if __name__ == '__main__':
    t1 = threading.Thread(target = task1)
    t2 = threading.Thread(target = task2)
    #t3 = threading.Thread(target = task3)
    t2.start()
    t1.start()
    #t3.start()
    #t2.join()
    
    
