from bluepy import btle

class MyDelegate(btle.DefaultDelegate):
    def __init__(self):
        btle.DefaultDelegate.__init__(self)
        # ... initialise here

    def handleNotification(self, cHandle, data):
        print("\n- handleNotification -\n")
        print(data)
        # ... perhaps check cHandle
        # ... process 'data'


# Initialisation  -------

p = btle.Peripheral("04:A3:16:A1:A6:86")
p.setDelegate( MyDelegate() )

# Setup to turn notifications on, e.g.
svc = p.getServiceByUUID("0000ffe0-0000-1000-8000-00805f9b34fb")
ch = svc.getCharacteristics("0000ffe1-0000-1000-8000-00805f9b34fb")[0]
#   ch.write( setup_data )

setup_data = b"\x01\00"
p.writeCharacteristic(ch.valHandle+1, setup_data)

# Main loop --------

while True:
    if p.waitForNotifications(1.0):
        # handleNotification() was called
        continue

    print("Waiting...")
    # Perhaps do something else here
