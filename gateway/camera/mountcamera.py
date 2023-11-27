#mount sd /media/video
# to init name sd when startup


from time import sleep
import os
import sys


# read file txt get name config
def get_name_sd():
    output = os.popen('df -h | awk \'$6 ~ /\/media\// {print $6}\'').read() # command mounted
    output = output.strip('\n')
    return output
def get_name_lsblk():
    os.system('sudo mount /dev/mmcblk2p1 /media/video')
get_name_lsblk()
