# -*- coding: utf-8 -*-
import socket
import serial
import requests
import time
import json
import signal
import sys
from threading import Lock

# 기존 URL 및 헤더 설정
apm2_url = 'http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/data'
data_send_url = 'http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/motor'
rpm_url = "http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/RPMData"
FanSpeed_url = "http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/FanSpeed"

sejong_url = "http://114.71.220.59:2021/Mobius/SMLplatform/Sensor/train"

apm2_headers = {
    'Accept': 'application/json',
    'X-M2M-RI': '12345',
    'X-M2M-Origin': 'Sgw',
    'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

sejong_headers = {
    'Accept': 'application/json',
    'X-M2M-RI': '12345',
    'X-M2M-Origin': 'SKsensor',
    'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

# 시리얼 포트 및 보드레이트 설정
serial_port_0 = '/dev/ttyM1'
serial_port_1 = '/dev/ttyM2'
serial_port_2 = '/dev/ttyM3'
serial_port_3 = '/dev/ttySensor'
serial_port_4 = '/dev/ttyNPM'

baudrate = 9600

# 시리얼 연결 초기화
try:
    ser0 = serial.Serial(serial_port_0, baudrate=baudrate, timeout=1)
    ser1 = serial.Serial(serial_port_1, baudrate=baudrate, timeout=1)
    ser2 = serial.Serial(serial_port_2, baudrate=baudrate, timeout=1)
    ser3 = serial.Serial(serial_port_3, baudrate=baudrate, timeout=1)
    ser4 = serial.Serial(serial_port_4, baudrate=baudrate, timeout=1)
    print(f'Connected to {ser0.name},{ser1.name},{ser2.name},{ser3.name},{ser4.name}')
except Exception as err:
    print(f"Serial error: {err}")
    sys.exit(1)
    
def clear_serial_buffer():
    ser0.reset_input_buffer()
    ser1.reset_input_buffer()
    ser2.reset_input_buffer()
    ser3.reset_input_buffer()
    ser4.reset_input_buffer()
    print("Serial buffers cleared")

first = 0
# 초기화 시 메시지 전송
if first == 0:
    ser0.write(b'0')
    ser1.write(b'0')
    ser2.write(b'0')
    ser3.write(b'0')
    ser4.write(b'0')
    first = 1
    clear_serial_buffer()
    print("loading....")
    time.sleep(5)

# 서버 설정
port = 65432

# 전역 변수 초기화
time_s, pm_data, pm_data_a = "", "", ""
running = True
is_restoring_fan_state = False

# 임계영역 보호를 위한 Lock 생성
lock = Lock()

def read_serial_data():
    try:
        # 각 시리얼 포트에 대해 데이터가 수신되었는지 확인
        if ser0.in_waiting > 0:
            data0 = ser0.readline().decode('utf-8').strip()
        else:
            data0 = ""

        if ser1.in_waiting > 0:
            data1 = ser1.readline().decode('utf-8').strip()
        else:
            data1 = ""

        if ser2.in_waiting > 0:
            data2 = ser2.readline().decode('utf-8').strip()
        else:
            data2 = ""

        if ser3.in_waiting > 0:
            data3 = ser3.readline().decode('utf-8').strip()
        else:
            data3 = ""

        if ser4.in_waiting > 0:
            data4 = ser4.readline().decode('utf-8').strip()
        else:
            data4 = ""

        # 수신된 데이터를 포맷팅하여 반환
        data = "{}{}{}{},{}".format(data0, data1, data2, data3, data4)
        if data0 == " , " or data1 == " , " or data2 == " , " or data3 == " , " or data4 == " , ":
            return "error"
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

# 750, 1200, 1650, 2100, 2550, 3060, 3420  
def send_sejong_data(data, url):
    where_list = data.split(',')
    
    data_apm = json.dumps({
        "m2m:cin": {
            "con": data
        }
    })
    
    
    if where_list[3] == '900':
        new_url = url + "/param1"
    elif where_list[3] == '1200':
        new_url = url + "/param2"
    elif where_list[3] == '1650':
        new_url = url + "/param3"
    elif where_list[3] == '2100':
        new_url = url + "/param4"
    elif where_list[3] == '2550':
        new_url = url + "/param5"        
    elif where_list[3] == '3060':
        new_url = url + "/param6"
    elif where_list[3] == '3420':
        new_url = url + "/param7"
    else:
        print("fanSpee error")

    try:
        r_apm2 = requests.post(new_url, headers=sejong_headers, data=data_apm)
        r_apm2.raise_for_status()
    except requests.exceptions.RequestException as req_err:
        print("Sejong request error:", req_err)
    except requests.exceptions.HTTPError as http_err:
        print("Sejong HTTP error:", http_err)
    except Exception as exc:
        print(f'Sejong problem occurred: {exc}')

def process_data(data):
    global time_s, pm_data, pm_data_a, is_restoring_fan_state

    with lock:  # 임계영역 시작
        if data.startswith('M'):
            if is_restoring_fan_state:
                # 복원 중일 때 데이터를 쓰기만 하고, 읽기는 생략
                if data[1] == '1':
                    ser0.write(data.encode('utf-8'))
                elif data[1] == '2':
                    ser1.write(data.encode('utf-8'))
                elif data[1] == '3':
                    ser2.write(data.encode('utf-8'))
                print(f"Data written during restore: {data}")
                clear_serial_buffer()
            else:
                clear_serial_buffer()
                if data[1] == '1':
                    ser0.write(data.encode('utf-8'))
                    time.sleep(10)
                    FanSpeed_data = ser0.readline().decode('utf-8').strip() if ser0.in_waiting > 0 else ""
                elif data[1] == '2':
                    ser1.write(data.encode('utf-8'))
                    time.sleep(10)
                    FanSpeed_data = ser1.readline().decode('utf-8').strip() if ser1.in_waiting > 0 else ""
                elif data[1] == '3':
                    ser2.write(data.encode('utf-8'))
                    time.sleep(10)
                    FanSpeed_data = ser2.readline().decode('utf-8').strip() if ser2.in_waiting > 0 else ""

                print(f"Received Fanspeed data: {FanSpeed_data}")
                send_apm_data(FanSpeed_data, FanSpeed_url)

        elif data.startswith('2'):
            retry = True
            while retry:
                retry = False
                rpm_data_list = data.split(',')
                time_s, pm_data, pm_data_a = rpm_data_list[0], rpm_data_list[5], rpm_data_list[6]
            
                print(f"Data starts with '2': {time_s},{pm_data},{pm_data_a}\n")
                clear_serial_buffer()
                print(f"sending start")
                ser0.write(b'start')
                ser1.write(b'start')
                ser2.write(b'start')
                ser3.write(b'start')
                ser4.write(b'start')
                time.sleep(5)
            
                con0_1 = read_serial_data()
                if "error" in con0_1:
                    clear_serial_buffer()
                    print("Error detected, resending 'start' signal")
                    retry = True
                else:
                    if time_s and pm_data and pm_data_a:
                        combined_data = "{},{},{},{}".format(time_s, con0_1, pm_data, pm_data_a)
                        print(f"{combined_data}")

                        data_list = combined_data.split(',')
                        original_order = [
                            "datetime", "pwm1_1", "pwm1_2", "pwm1_3", "pm1_1", "pm1_2", "pm1_3",
                            "pwm2_1", "pwm2_2", "pwm2_3", "pm2_1", "pm2_2", "pm2_3",
                            "pwm3_1", "pwm3_2", "pwm3_3", "pm3_1", "pm3_2", "pm3_3",
                            "temp", "humi", "o3", "co", "no2", "so2",
                            "wind_d", "wind_s", "npm", "rpm before correction", "rpm after correction"
                        ]
                        desired_order = [
                            "datetime", "rpm before correction", "rpm after correction",
                            "pwm1_1", "pwm1_2", "pwm1_3",
                            "pwm2_1", "pwm2_2", "pwm2_3",
                            "pwm3_1", "pwm3_2", "pwm3_3",
                            "npm",
                            "pm1_1", "pm1_2", "pm1_3",
                            "pm2_1", "pm2_2", "pm2_3",
                            "pm3_1", "pm3_2", "pm3_3",
                            "temp", "humi", "o3", "co",
                            "no2", "so2", "wind_d", "wind_s"
                        ]

                        if len(data_list) != len(original_order):
                            clear_serial_buffer()
                            print("Data length mismatch: Retrying.")
                            retry = True
                        else:
                            index_map = {original_order[i]: i for i in range(len(original_order))}
                            reordered_data = [data_list[index_map[col]] for col in desired_order]
                            combined_data = ','.join(reordered_data)
                            print(f"Reordered data: {combined_data}")
                            send_apm_data(combined_data, apm2_url)
                            send_sejong_data(combined_data, sejong_url)
                    else:
                        print("Global variables are not yet initialized.")  
     

def server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('localhost', port))
        s.listen()
        print(f"Listening on port {port}...")

        while running:
            conn, addr = s.accept()
            with conn:
                print(f'Connected to {addr}')
                while running:
                    data = conn.recv(1024)
                    if not data:
                        break
                    data = data.decode('utf-8').strip()
                    process_data(data)

def signal_handler(sig, frame):
    global running
    print('Safely shutting down...')
    running = False
    ser0.close()
    ser1.close()
    ser2.close()
    ser3.close()
    ser4.close()
    
    sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)

# 서버 실행
try:
    server()
except Exception as e:
    print(f"Server error: {e}")
finally:
    ser0.close()
    ser1.close()
    ser2.close()
    ser3.close()
    ser4.close()
    print("Serial connection closed.")
