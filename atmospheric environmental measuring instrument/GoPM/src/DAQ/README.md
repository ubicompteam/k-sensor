# DAQ

## Language Selection

[English](README.md) | [한국어](README_KR.md)

<br><br>

## Code Overview

- This code collects data every minute from the Jetson Nano and uploads the values to the Mobius server.
- It is also designed to receive real-time user commands via MQTT, allowing for FAN speed control.

<br><br>

## Main Code Details

### URL Configuration

```
apm2_url = 'http://114.71.220.59:2021/Mobius/gw/data'
data_send_url = 'http://114.71.220.59:2021/Mobius/gw/motor'
rpm_url = "http://114.71.220.59:2021/Mobius/gw/RPMData"
FanSpeed_url = "http://114.71.220.59:2021/Mobius/gw/FanSpeed"
```

- **apm2_url**: Container for collecting and uploading data from reference equipment and GoPM modules.
- **data_send_url**: Container where commands to control the fan speed of individual modules are sent (MQTT Broker).
- **rpm_url**: Container where reference equipment data (ground truth) is uploaded.
- **FanSpeed_url**: Container for receiving feedback on whether the controlled modules have reached the specified speed.


<br>

### Data Request Function

```
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
```

- This function retries data requests from the Arduino in case data isn't received. 
- If no response is received after retrying, it returns `"null"` to indicate an error.

<br>

### Feedback Request Function

```
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
```

- This function retries fan speed control if feedback is not received after the first attempt.
- If there is still no feedback, it prompts a physical check on the fan, likely on the 7th floor, indicating a potential issue.

<br>

### Fan Speed Control Function

```
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
```

- This function performs fan speed control based on user commands sent to `data_send_url`.
- It uses a critical section to avoid conflicts between data collection and command execution.
- The command is sent to the Arduino to control fan speed, and if feedback is not received, it triggers the feedback request function.

<br>

### Periodic Data Collection Function

```
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

```

- This function executes every minute, fetching data from `rpm_url`, gathering data from the Arduino, and uploading the combined data to `apm2_url`.
- It uses a critical section to prevent conflicts between data collection and command execution.
- The data is formatted and reordered as needed before being sent to the server.

<br>

### Server Function

```
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
```

- This function starts separate threads for handling command reception and the periodic data collection.
- It configures TCP settings to receive values sent via MQTT.
