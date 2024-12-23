# DAQ

## 언어 선택

[English](README.md) | [한국어](README_KR.md)

<br><br>

## 코드 설명

- 해당 코드는 젯슨나노 기준 1분마다 데이터를 수집하여 모비우스 서버에 값을 올리는 일을함
- 또한 MQTT를 사용하여 사용자 명령을 실시간으로 받아 FAN속도를 제어할 수 있게끔 설계되었음

<br><br>

## 주요 코드 상세 설명

```
apm2_url = 'http://114.71.220.59:2021/Mobius/gw/data'
data_send_url = 'http://114.71.220.59:2021/Mobius/gw/motor'
rpm_url = "http://114.71.220.59:2021/Mobius/gw/RPMData"
FanSpeed_url = "http://114.71.220.59:2021/Mobius/gw/FanSpeed"
```

- apm2_url : 레퍼런스 장비의 데이터와 GoPM, 즉 모듈들이 측정한 값을 모아 데이터를 올리는 컨테이너
- data_send_url : 사용자가 각각의 모듈들의 팬 속도를 제어할 때 명령이 올라가는 컨테이너 ( MQTT Broker )
- rpm_url : 레퍼런스 장비의 측정 값이 올라가는 컨테이너 ( 레퍼런스, 즉 우리한테는 정답지 )
- FanSpeed_url : 사용자가 모듈들을 제어하고 실제로 그 모듈들이 조절한 속도로 도달했는지에 대한 피드백을 받는 컨테이너

<br>

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

- 해당 함수는 아두이노에서 가끔 데이터를 수신하지 못하는 경우가 발생할 경우 다시 한번 더 요청하는 함수임
- 만약 재시도 했을 경우에도 응답이 없으면 null을 넣어 응답 오류를 나타냄 (데이터는 계속 수집되야 함)

<br>

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

- 해당 함수는 아두이노에서 가끔 팬 속도를 제어 후 피드백이 오지 않을 경우 한번 더 팬을 제어하여 피드백을 얻는 함수임
- 만약 재시도 했을 경우에도 응답이 없으면 팬에 문제가 생겼을 확률이 높으므로 설치된 장소(7층)에 올라가 확인을 해야함

<br>

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

- 해당 함수는 사용자가 팬 제어를 data_send_url 으로 명령을 내렸을 떄 그 명령을 수행하는 함수임
- 임계영역을 주어 데이터 수집하는 시간과 명령 제어와 충돌하지 않게 설정해둠
- 받은 값을 아두이노로 전송해 팬을 제어하고, 피드백이 오지않으면 try_send_feedback함수를 실행시킴

<br>

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

- 해당 함수는 1분마다 실행되어 rpm_url에서 데이터를 가져오고, 아두이노로 부터 데이터를 가져와 통합하여 apm2_url으로 데이터를 전송하는 함수임
- 임계영역을 주어 데이터 수집하는 시간과 명령 제어와 충돌하지 않게 설정해둠
- 데이터 수집을 하고 우리가 사용하는 데이터에 맞게 정렬함

<br>

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

- 해당 함수는 명령어 받는 스레드와 1분마다 실행되는 스레드를 실행시키고, mqtt에서 날라오는 값을 tcp받기위헤 tcp설정을 하는 함수임

