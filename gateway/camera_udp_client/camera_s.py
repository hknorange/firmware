#camera back
# usb 2.1
#header 3
import socket,cv2, pickle,struct
import pyshine as ps # pip install pyshine
import imutils # pip install imutils
from time import sleep
import os
import sys
from datetime import datetime
from flask import Flask, Response, render_template
import threading
import json
import base64

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
count_out = 0
while(name_card == ""):
    name_card = get_name_sd()
    if((name_card != "") or count_out == 10):
        break
    count_out+=1
    sleep(1)

cap = cv2.VideoCapture(usb_port)
cap.set(cv2.CAP_PROP_FPS,10)
size_frame = (640,480)

if(cap.isOpened()):
    now = datetime.now()
    date_time = now.strftime('back_date_%d_%m_%Y time_%H_%M_%S')
    path_save = os.path.join(name_card,date_time)
    print(path_save)
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
    s=socket.socket(socket.AF_INET , socket.SOCK_DGRAM)  # Gives UDP protocol to follow
    s.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 100000000) # setSockoptTo open two protocols,SOL_SOCKET: Request applies to socket layer.
    port = 2323
    client_socket = (host_ip,port)
    while True:
        success, frame = cap.read()
        now = datetime.now()
        date_time = now.strftime('%d-%m-%Y %H:%M:%S')
        cv2.putText(frame,date_time,(5,20),cv2.FONT_HERSHEY_PLAIN,1.5,(40,0,0),1,cv2.LINE_AA)
        #frame =  cv2.resize(frame,(640,480),interpolation = cv2.INTER_LINEAR)
        #cv2.imshow("abc",frame)
        #display_frame = frame
        result.write(frame)
        success,tframe = cv2.imencode(".jpg", frame, [int(cv2.IMWRITE_JPEG_QUALITY),30])
        frame_data = base64.b64encode(tframe)
        message = {
            'license_car': car_ip,
            'position_camera': '3',
            'frame_data': frame_data.decode('utf-8')
        }
        json_message = json.dumps(message)

        s.sendto(json_message.encode('utf-8'),client_socket) 

        #cv2.imshow("abc",frame)
        #print('frame',frame)

        if (cv2.waitKey(1) == ord('q')):
            cv2.destroyAllWindows()
            break; 
        #print(frame)
      
    
if __name__ == '__main__':
    #t1 = threading.Thread(target = task1)
    t2 = threading.Thread(target = task2)
    #t3 = threading.Thread(target = task3)
    t2.start()
    #t1.start()
    #t3.start()
    #t2.join()
    
    
