import subprocess

result = subprocess.run(["sudo","v4l2-ctl","--list-devices"],capture_output=True,text=True)
print("return\r\n")
a = (result.stdout)

CAMERA = "UNIQUESKY_CAR_CAMERA"
USB3_0 = "usb-xhci-hcd.1.auto-1" #camera lui
USB2_0_2 = "usb-fe380000.usb-1"    #camera trước
USB2_0_1 = "usb-fe3c0000.usb-1"    #camera tx

b = a.split("\n\t")
print(b)

pos = []
keywords = [USB3_0,USB2_0_1,USB2_0_2]

n1=n2=n3=0
for key in keywords:
    for i,item in enumerate(b):
        if(key in item):
            if(key == USB3_0):
                n1 = i;
            elif (key == USB2_0_1):
                n2=i
            elif (key == USB2_0_2):
                n3=i
with open("/root/wiringOP/code_220923_gateway/portusbcamera/port_usb_3_0.txt","w") as file:
    if(n1 != 0):
        file.write(b[n1+1])
        print(b[n1+1])
    else:
        file.write("no find")
with open("/root/wiringOP/code_220923_gateway/portusbcamera/port_usb_2_0_0.txt","w") as file:
    if(n2 != 0):
        file.write(b[n2+1])
        print(b[n2+1])
    else:
        file.write("no find")
        
with open("/root/wiringOP/code_220923_gateway/portusbcamera/port_usb_2_0_1.txt","w") as file:
    if(n3 != 0):
        file.write(b[n3+1])
        print(b[n3+1])
    else:
        file.write("no find")
    



