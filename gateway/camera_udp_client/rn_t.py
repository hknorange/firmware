import socket
import subprocess
import time

def check_wifi_connection():
    host = "www.google.com"
    port = 80
    timeout = 5  # Thời gian chờ kết nối trong giây

    try:
        # Tạo một đối tượng socket
        socket.create_connection((host, port), timeout=timeout)
        print("Đã kết nối Wi-Fi.")
        return True
    except (socket.error, socket.timeout):
        print("Chưa kết nối Wi-Fi.")
        return False

def run_program():
    # Thay thế bằng lệnh hoặc hàm chạy chương trình Python của bạn
    program_path = "/root/Documents/camera_udp_client/camera_t.py"
    subprocess.run(['python3', program_path])

def main():
    while True:
        if check_wifi_connection():
            # Nếu đã kết nối Wi-Fi, chạy chương trình Python
            print("Thực hiện các công việc khác khi đã kết nối Wi-Fi.")
            run_program()
        
        # Chờ 1 phút trước khi kiểm tra lại
        time.sleep(10)

if __name__ == "__main__":
    main()
