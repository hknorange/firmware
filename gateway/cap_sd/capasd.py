import psutil
import os
import time
sd_space = False
name_sd = ""
def get_sd_card_space():
    global sd_space
    disk_partitions = psutil.disk_partitions(all=True)
    
    for partition in disk_partitions:
        if 'mmcblk2p1' in partition.device or 'sd' in partition.device:
            usage = psutil.disk_usage(partition.mountpoint)
            total_space_gb = round(usage.total / (1024 ** 3), 2)
            used_space_gb = round(usage.used / (1024 ** 3), 2)
            free_space_gb = round(usage.free / (1024 ** 3), 2)
            print("a:",total_space_gb - used_space_gb)
            if((total_space_gb - used_space_gb) < 1):
                sd_space = True
                
            
            print(f"Partition: {partition.device}")
            print(f"Total Space: {total_space_gb} GB")
            print(f"Used Space: {used_space_gb} GB")
            print(f"Free Space: {free_space_gb} GB\n")

def get_name_sd():
    output = os.popen('df -h | awk \'$6 ~ /\/media\// {print $6}\'').read() 
    output = output.strip('\n')
    return output
def keep_latest_files(directory, files_to_keep):
    all_files = [f for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f))]
    all_files.sort(key=lambda f: os.path.getmtime(os.path.join(directory, f)))

    files_to_delete = all_files[:-files_to_keep]  # Lấy các tệp tin cũ trừ số file bạn muốn giữ lại

    for file_to_delete in files_to_delete:
        file_path = os.path.join(directory, file_to_delete)
        try:
            os.remove(file_path)
            print(f"Deleted old file: {file_path}")
        except Exception as e:
            print(f"Error deleting file {file_path}: {e}")
            

if __name__ == "__main__":
    
    files_to_keep = 3  # Số file bạn muốn giữ lại
    name_sd = get_name_sd()
    while (name_sd == ""):
        sleep(1)
        name_sd = get_name_sd()
        if(name_sd != ""):
            break
    print('sd_space:',sd_space)
    get_sd_card_space()
    print('sd_space1:',sd_space)
    print('sd_space1:',name_sd)
    if(sd_space == True):
        keep_latest_files(name_sd, files_to_keep)
