import os
output = []
def get_name_sd():
    b = os.popen('df -h | awk \'$6 ~ /\/media\// {print $2}\'').read() # command mounted
    b = b.strip('G\n')
    output.append(b)
    b = os.popen('df -h | awk \'$6 ~ /\/media\// {print $3}\'').read() # command mounted
    b = b.strip('G\n')
    output.append(b)
    b = os.popen('df -h | awk \'$6 ~ /\/media\// {print $4}\'').read() # command mounted
    b = b.strip('G\n')
    output.append(b)
    
    return output
a = get_name_sd()


a = [float(i) for i in a]
print(a)
if((a[0] - a[2]) > 1):
    print(a[0] - a[1])
else:
    print(a[0] - a[1])


