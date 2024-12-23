import requests
import time
from datetime import datetime, timedelta


mobius_url = 'http://114.71.220.59:7579/Mobius/Ksensor_ubicomp/motor'  # Mobius 서버 주소와 요청 경로 설정
headers = {
  'Accept': 'application/json',
  'X-M2M-RI': '12345',
  'X-M2M-Origin': 'SJwrKvZd84v',
  'Content-Type': 'application/vnd.onem2m-res+json;ty=4'
}

"""
mobius_url = 'http://203.253.128.177:7579/Mobius/PCY_Home/Value'
headers = {
    'Accept': 'application/json',
    'X-M2M-RI': '12345',
    'X-M2M-Origin': 'SZ8V742I5tw',
    'Content-Type': 'application/vnd.onem2m-res+json;ty=4'
}
"""

# 각 배열 정의
param1 = ["pm1_1", "pm2_1", "pm3_1", "bottom_0"]
param2 = ["pm1_2", "pm2_2", "pm3_2", "bottom_0"]
param3 = ["pm1_3", "pm2_3", "pm3_3", "bottom_0"]
param4 = ["pm1_1", "pm2_1", "pm3_1", "bottom_30"]
param5 = ["pm1_2", "pm2_2", "pm3_2", "bottom_30"]
param6 = ["pm1_3", "pm2_3", "pm3_3", "bottom_30"]
param7 = ["pm1_1", "pm2_1", "pm3_1", "bottom_60"]
param8 = ["pm1_2", "pm2_2", "pm3_2", "bottom_60"]
param9 = ["pm1_3", "pm2_3", "pm3_3", "bottom_60"]
param10 = ["pm1_1", "pm2_1", "pm3_1", "bottom_90"]
param11 = ["pm1_2", "pm2_2", "pm3_2", "bottom_90"]
param12 = ["pm1_3", "pm2_3", "pm3_3", "bottom_90"]

params_list = [param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12]

def send_params(params):
    for param in params:
        # Mobius 서버에 POST 요청 보내기
        print(param)
        data = "{\n    \"m2m:cin\": {\n        \"con\": \"" + param + "\"\n    }\n}"
        response = requests.request("POST", mobius_url, headers=headers, data=data)

        # # 응답 확인 및 오류 처리
        if response.status_code == 201:
            print("명령을 성공적으로 전송했습니다.")
        else:
            print(f"오류 발생: {response.status_code} - {response.text}")

        # 10초 대기
        time.sleep(10)


# 2시간 단위로 반복
while True:
    for params in params_list:
        print(params)
        print(datetime.now())
        send_params(params)

        # 2시간 대기
        time.sleep(2 * 60 * 60)  # 2시간 대기 (초 단위)
        #time.sleep(60)


"""
# 현재 시간 가져오기
current_time = datetime.now()

# 다음 00시까지 대기
target_time = datetime(current_time.year, current_time.month, current_time.day, 0, 0)
if current_time > target_time:
    target_time += timedelta(days=1)

time_to_wait = target_time - current_time
time.sleep(time_to_wait.total_seconds())

"""
