from datetime import datetime, timedelta
import threading
import serial
import requests
import time

port0 = '/dev/ttyACM1'
port1 = '/dev/ttyACM0'
#NPM
port2 = '/dev/ttyACM3'

url = 'http://114.71.220.59:7579/Mobius/Ksensor_ubicomp'

brate0 = 9600
brate1 = 9600
brate2 = 9600

headers = {
  'Accept': 'application/json',
  'X-M2M-RI': '12345',
  'X-M2M-Origin': 'SJwrKvZd84v',
  'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

try:
    ser0 = serial.Serial(port0, baudrate=brate0, timeout=None)
    ser1 = serial.Serial(port1, baudrate=brate1, timeout=None)
    ser2 = serial.Serial(port2, baudrate=brate2, timeout=None)
    print('%s and %s and %s' %(ser2.name, ser0.name, ser1.name))
except Exception as err:
    print("Serial err:", err)


def calculate_next_run_time():
    now = datetime.now()
    next_run = now.replace(second=0, microsecond=0) + timedelta(minutes=(5 - now.minute % 5))
    if next_run.minute % 5 != 0:
        next_run += timedelta(minutes=(5 - next_run.minute % 5))
    return next_run

def print_current_time():
    global next_run
    current_time = datetime.now()

    send_s = 'start'
    send_S = send_s.encode('utf-8')
    ser0.write(send_S)
    ser1.write(send_S)
    ser2.write(send_S)

    next_run = calculate_next_run_time()
    delay = (next_run - datetime.now()).total_seconds()
    threading.Timer(max(0, delay), print_current_time).start()

def print_two():
    while True:
        time.sleep(2)
	if ser0.in_waiting != 0 and ser1.in_waiting != 0 and ser2.in_waiting != 0:
            current_time = datetime.now().strftime('%Y%m%d%H%M%S')
            try: 
                contect2 = ser2.readline()
                contect0 = ser0.readline()
                contect1 = ser1.readline()
            
                con0_1 = "{},{},{},{}".format(current_time, contect2[:-2].decode(), contect0[:-2].decode(), contect1[:-2].decode())
            
            except Exception as err:
                print("Serial read err:", err)

            con0_1 = con0_1.replace("\n", "")
            print(con0_1)
        
            data = "{\n    \"m2m:cin\": {\n        \"con\": \"" + con0_1 + "\"\n    }\n}"

            try:
                r = requests.request("POST", url+"/data", headers=headers, data=data)
                r.raise_for_status()
                jr = r.json()
            except requests.exceptions.RequestException as req_err:
                print("Request error:", req_err)
            except requests.exceptions.HTTPError as http_err:
                print("HTTP error:", http_err)
            except Exception as exc:
                print('There was a problem: %s' % (exc))
        
next_run = calculate_next_run_time()
threading.Thread(target=print_current_time).start()
threading.Thread(target=print_two).start()
