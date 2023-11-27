dir:
	portusbcamera: cmd v4l-ctl --list-devices lấy các port usb of camera
	websocketble: code gửi websocket to server & nhận data sensor from ble & nhận pipe fifo from gps
	gps: nhận data gps & gửi pipe fifo to websocketble
