import os
thu_muc = '/media/root/8A15-04F1'
ten_thu_muc = os.listdir(thu_muc)
with_time = [(item,os.path.getctime(os.path.join(thu_muc,item))) for item in ten_thu_muc]
sortd = sorted(with_time,key =lambda x: x[1])

    
print((sortd[0]))