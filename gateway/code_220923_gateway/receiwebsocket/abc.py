import os
PATHPIPE = '/root/wiringOP/code_220923_gateway/receiwebsocket/node'
def pipe_recei():
	global pipe_data
	while True:
		with open(PATHPIPE,"r") as pipe:
			pipe_data = pipe.read()
			print("recei pipe:" + pipe_data)
pipe_recei()