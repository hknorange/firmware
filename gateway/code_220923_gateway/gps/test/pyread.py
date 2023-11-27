import os
myfifo = "/tmp/myfifo"
#os.mkfifo(myfifo)
data=""
while True:
	with open(myfifo,"r") as pipe:
		data = pipe.read()
		print(data)
