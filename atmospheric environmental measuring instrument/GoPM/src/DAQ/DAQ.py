import socket
from datetime import datetime
import serial
import requests
import time
import json
import signal
import sys
import threading

# 기존 URL 및 헤더 설정
apm2_url = 'http://114.71.220.59:2021/Mobius/gw/data'
data_send_url = 'http://114.71.220.59:2021/Mobius/gw/motor'
rpm_url = "http://114.71.220.59:2021/Mobius/gw/RPMData"
FanSpeed_url = "http://114.71.220.59:2021/Mobius/gw/FanSpeed"

apm2_headers = {
    'Accept': 'application/json',
    'X-M2M-RI': '12345',
    'X-M2M-Origin': 'Sgw',
    'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

# 시리얼 포트 및 보드레이트 설정
serial_port_0 = '/dev/ttyACM0'
baudrate = 9600

# 시리얼 연결 초기화
try:
    ser0 = serial.Serial(serial_port_0, baudrate=baudrate, timeout=1)
    print(f'Connected to {ser0.name}.')
except Exception as err:
    print(f"Serial error on {serial_port_0}: {err}")
    sys.exit(1)
    
def clear_serial_buffer():
    ser0.reset_input_buffer()
    print("Serial buffers cleared")

first = 0
# 초기화 시 메시지 전송
if first == 0:
    ser0.write(b'0')
    first = 1
    clear_serial_buffer()
    print("loading....")
    time.sleep(5)

# 서버 설정
port = 12345

# 전역 변수 초기화
running = True

# 임계영역 보호를 위한 Lock 생성 ( 데이터 수집과 제어 명령어 전송과의 충돌 방지 )
lock = threading.Lock()

# 아두이노에서 데이터 수신 오류시 재시도 함수    
def try_send_data(try_serial_port):
    try:
        clear_serial_buffer(try_serial_port)
        try_serial_port.write(b'start')
        time.sleep(5)
        data = try_serial_port.readline().decode('utf-8').strip()
        if data == "":
            return "null"
        return data
    except Exception as try_err:
        print(f"Serial send error: {try_err}")
        return "null"
    
# 아두이노에서 피드백 수신 오류시 재시도 함수
def try_send_feedback(try_serial_port, data):
    try:
        clear_serial_buffer(try_serial_port)
        try_serial_port.write(data.encode('utf-8'))
        time.sleep(10)
        data = try_serial_port.readline().decode('utf-8').strip()
        if data == "":
            return "check the fan go to 7 floor"
        return data
    except Exception as try_err:
        print(f"Serial send error: {try_err}")
        return "check the fan go to 7 floor"

# 시리얼 데이터 수신 함수
def read_serial_data():
    try:
        # 각 시리얼 포트에 대해 데이터가 수신되었는지 확인
        if ser0.in_waiting > 0:
            data0 = ser0.readline().decode('utf-8').strip()
        else:
            # 만약 데이터가 없다면 재시도
            data0 = try_send_data(ser0)

        # 수신된 데이터를 포맷팅하여 반환
        data = "{}".format(data0)
        if data0 == " , " :
            return "null"
        elif data == "null":
            return "null"
        return data
    except Exception as read_err:
        print(f"Serial read error: {read_err}")
        return "error"

def send_apm_data(data, url):
    data_apm = json.dumps({
        "m2m:cin": {
            "con": data
        }
    })

    try:
        r_apm2 = requests.post(url, headers=apm2_headers, data=data_apm)
        r_apm2.raise_for_status()
    except requests.exceptions.RequestException as req_err:
        print("APM request error:", req_err)
    except requests.exceptions.HTTPError as http_err:
        print("APM HTTP error:", http_err)
    except Exception as exc:
        print(f'APM problem occurred: {exc}')
        
# 팬 속도 제어 함수        
def CommandFanControl(data):
    
    lock.acquire()
    clear_serial_buffer()
    if data[1] == '1':                  # 모듈 번호 1
        ser0.write(data.encode('utf-8'))
        time.sleep(10)                  # fanSpeed가 명령한 speed에 도달하고 피드백을 주기위한 시간 지연
        # 피드백이 없는 경우 다시 명령어를 보내 피드백을 받기 위해 시도, 10초 후에도 없으면 "check the fan go to 7 floor" 반환
        FanSpeed_data = ser0.readline().decode('utf-8').strip() if ser0.in_waiting > 0 else try_send_feedback(ser0, data)

    print(f"Received Fanspeed data: {FanSpeed_data}")
    send_apm_data(FanSpeed_data, FanSpeed_url)

    print(f"lock released")
    lock.release()

# 1분마다 실행되는 함수
def OneMinute(url):
    global RPM_pm_data_b, RPM_pm_data_a, RPM_time

    print("Acquiring lock")
    lock.acquire()
    
    try:
        # APM2 시간 수집
        APM2_time = datetime.now().strftime('%Y%m%d%H%M%S')
        
        # RPM 데이터 수집
        print("Fetching RPM data")
        rpm_data_get = requests.get(url, headers=apm2_headers)
        rpm_data_get.raise_for_status()
        rpm_data_get_json = rpm_data_get.json()
        rpm_data_original = rpm_data_get_json.get("m2m:cin", {}).get("con", None)
        print(f"RPM data fetched: {rpm_data_original}")

        # RPM 데이터 처리
        if rpm_data_original:
            rpm_data_original_list = rpm_data_original.split(',')
            RPM_time = rpm_data_original_list[0] if len(rpm_data_original_list) > 0 else 'null'
            RPM_pm_data_b = rpm_data_original_list[5] if len(rpm_data_original_list) > 5 else 'null'
            RPM_pm_data_a = rpm_data_original_list[6] if len(rpm_data_original_list) > 6 else 'null'
        else:
            RPM_time = RPM_pm_data_b = RPM_pm_data_a = 'null'

    # APM2 데이터 수집 및 전송
        clear_serial_buffer()
        print("Sending start command to serial ports")
        for ser in [ser0]:
            ser.write(b'start')
        time.sleep(5)
        
        APM2_Data = read_serial_data()
        sensor_data = "0.0,0.0,0.000,0.000,0.000,0.000,0.0,0.0,0"
        combined_data = f"{APM2_time},{RPM_time},{APM2_Data},{sensor_data},{RPM_pm_data_b},{RPM_pm_data_a}"
        print(f"Combined data: {combined_data}")
        
        # 데이터 재정렬
        original_order = [
            "APM_datetime", "RPM_datetime", "pwm1_1", "pwm1_2", "pwm1_3", "pm1_1", "pm1_2", "pm1_3", 
            "temp", "humi", "o3", "co", "no2", "so2",
            "wind_d", "wind_s", "npm", "rpm before correction", "rpm after correction"
        ]
        
        desired_order = [
            "APM_datetime", "RPM_datetime", "rpm before correction", "rpm after correction",
            "pwm1_1", "pwm1_2", "pwm1_3", "npm", "pm1_1", "pm1_2", "pm1_3",
            "temp", "humi", "o3", "co", "no2", "so2", "wind_d", "wind_s"
        ]

        index_map = {original_order[i]: i for i in range(len(original_order))}
        data_list = combined_data.split(',')
        reordered_data = [data_list[index_map[col]] for col in desired_order]
        combined_data = ','.join(reordered_data)
        print(f"Reordered data: {combined_data}")
        
        # APM2서버로 데이터 전송
        send_apm_data(combined_data, apm2_url)
    
    except Exception as exc:
        print(f"Error occurred: {exc}")
    finally:
        print("Releasing lock")
        lock.release()

# TCP통신 함수
def handle_connection(conn, addr):
    print(f'Connected to {addr}')
    while True:
        data = conn.recv(1024)
        #print(data)
        if not data:
            break
        data = data.decode('utf-8').strip()
        if data.startswith('M'):
            CommandFanControl(data)
    conn.close()

# 스케줄링 함수
def schedule_tasks():
    while True:
        time.sleep(60)
        print("time's up")
        OneMinute(rpm_url + '/la')
        
# 서버 함수
def server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('localhost', port))
        s.listen()
        print(f"Listening on port {port}...")

        # 스케줄 작업을 별도의 스레드에서 실행
        scheduler_thread = threading.Thread(target=schedule_tasks, daemon=True)
        scheduler_thread.start()

        while True:
            conn, addr = s.accept()
            client_thread = threading.Thread(target=handle_connection, args=(conn, addr))
            client_thread.start()

# 키보드 인터럽트 핸들러     
def signal_handler(sig, frame):
    global running
    print('Safely shutting down...')
    running = False
    ser0.close()
    sys.exit(0)

# 키보드 인터럽트 핸들러 등록
signal.signal(signal.SIGINT, signal_handler)

clear_serial_buffer()

# 서버 실행
try:
    server()
except KeyboardInterrupt:
    print("KeyboardInterrupt")
    ser0.close()
    sys.exit(0)
