# APM_v2

## 언어 선택

[English](README.md) | [한국어](README_KR.md)

<br><br>

## 개요

- 대기 오염 수집 장치는 저가형 미세먼지 센서를 이용하여 고가의 미세먼지 측정 장비와 비슷한 성능을 내고자 저가형 센서 기반으로 제작된 장비임
- 기상 및 대기 오염 물질과 고농도 미세먼지의 계절별 상관 분석 연구 결과를 바탕으로, 미세먼지, 온습도, 아황산가스, 일산화탄소, 이산화질소, 풍향, 풍속 센서가 설치됨
- 장치는 환경부의 대기 오염 측정 및 운영지침에 따라 1.5미터 이상의 높이에 센서들을 위치시킴
- 상부에는 측정을 위한 저가형 미세먼지 측정 센서와 환경 센서(온습도, 아황산가스, 일산화탄소, 이산화질소, 풍향, 풍속, 오존)가 설치됨
- 미세먼지 센서 내부의 팬을 제거한 후 PWM 제어 팬을 설치한 후 PWM 제어팬을 이용하여, 공기 유입량 제어가 가능한 형태로 제작됨

<br><br>


### 모델링 및 설치 사진
<div align="center">
  
  | 모델링 | 설치 |
  |:---:|:---:|
  | <img src="https://github.com/user-attachments/assets/ba80964b-d8c8-4775-a51b-e386a5ecab33" width="470px" height="350px" alt="모델링"> | <img src="https://github.com/user-attachments/assets/8cf09806-a0ee-473e-bbfc-5edc33816b9d" width="470px" height="350px" alt="설치"> |
</div>

<br><br>

### APM_v2 Mqtt(모터) 구동 영상

- 모터는 필요없다고 생각되어 현재 탈거하였음

[APM_v2 Mqtt 구동 영상 보기](https://youtu.be/hN8SpTdIn4Q?feature=shared)

<br><br>

### 디렉토리 형식

```
APM_v2/
├── Document/
│   ├── Guide_KR
│   └── Guide_EN
│ 
├── src/
│   ├── DAQ/
│   │   └── DAQ.py
│   │
│   ├── Mobius_server_mqtt/
│   │   └── nCube-Thyme-Nodejs.zip
│   │
│   ├── Sensor/
│   │   ├── Environmental_Sensor/
│   │   │   ├── DFRobot_OzoneSensor.cpp
│   │   │   ├── DFRobot_OzoneSensor.h
│   │   │   └── Environmental_Sensor.ino
│   │   ├── Module_Floor_1/
│   │   │   └── Module_Floor_1.ino
│   │   ├── Module_Floor_2/
│   │   │   └── Module_Floor_2.ino
│   │   ├── Module_Floor_3/
│   │   │   └── Module_Floor_3.ino
│   │   ├── NPM
│   │   │   └── NPM.ino

```

<br><br>

### 사용된 하드웨어 장비

| 장비명                | 사양                                                                                                                                                                                                                                                                                                                                                                     | 수량 | 비고                         |
|:------------------:|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:---:|:--------------------------:|
| Jetson Nano B 01   | Quad-core ARM® Cortex-A57@ 1.43 GHz                                                                                                                                                                                                                                                                                                                                    | 1  |                            |
| Arduino Mega 2560  | ATmega2560                                                                                                                                                                                                                                                                                                                                                             | 4  | 미세먼지 데이터 3, 환경 데이터 1       |
| Arduino Uno        | ATmega328P                                                                                                                                                                                                                                                                                                                                                             | 1  |                            |
| PMS7003            | 광산란 방식 <br> (fan 유무 : 무)                                                                                                                                                                                                                                                                                                                                               | 9  | 미세먼지 센서                    |
| 환경 센서              | CO, NO2, SO2 each 1 (AllSensing AGSM series)<br> Ozone 1 (sen0321) - I2C communication<br> WindSpeed 1 (sen0170) - Analog communication<br> WindDirection 1 (WS5029) - Analog communication<br> Temperature and Humidity (DHT22) - Digital                                                                                                             |    | UART, I2C, Digital, Analog |
| 구동부 센서             | Fan                                                                                                                                                                                                                                                                                                                                                                    | 9  | 4선 Fan                     |
| 기타                 | 파워서플라이 1개 (방수) <br>전기박스 358 x 270 x 152 1개 <br>전기박스 500 x 400 x 160 1개 <br>프로파일 40 x 40 x 500 6개 <br>프로파일 40 x 40 x 250 16개 <br>프로파일 20 x 80 x 400 8개 <br>프로파일 20 x 40 x 400 11개 <br>프로파일 20 x 40 x 360 8개 <br>프로파일 20 x 20 x 400 8개 <br>프로파일 20 x 20 x 470 10개 <br>SSEBL420 8개 <br>4선 실드 케이블 (약 1.5M) 9개 <br>2선 실드 케이블 (1.5M) 8개 <br>3D 프린터로 나머지 재료 제작 (PLA+ 재질)  |    |                            |


<br><br>

### 배선도

<div align="center">

  | 센서 배선 | 
  |:---:|
  | <img src="https://github.com/user-attachments/assets/c789b326-338c-4222-890a-6d37527918c3" width="600px" height="550px" alt="센서 배선도"> |
</div>

<br>

### 센서 배선도

<div align="center">
  <table>
    <tr>
      <td>
        <table border="1">
          <tr>
            <th><strong>Arduino Mega 2560 (환경센서)</strong></th>
            <th><strong>온습도, CO, NO2, SO2, O₃</strong></th>
          </tr>
          <tr>
            <td>5V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>D6</td>
            <td><strong>온습도_OUT</strong></td>
          </tr>
          <tr>
            <td>D19</td>
            <td><strong>CO_Tx</strong></td>
          </tr>
          <tr>
            <td>D18</td>
            <td><strong>CO_Rx</strong></td>
          </tr>
          <tr>
            <td>D17</td>
            <td><strong>NO2_Tx</strong></td>
          </tr>
          <tr>
            <td>D16</td>
            <td><strong>NO2_Rx</strong></td>
          </tr>
          <tr>
            <td>D15</td>
            <td><strong>SO2_Tx</strong></td>
          </tr>
          <tr>
            <td>D14</td>
            <td><strong>SO2_Rx</strong></td>
          </tr>
          <tr>
            <td>D20</td>
            <td><strong>O₃_SDA</strong></td>
          </tr>
          <tr>
            <td>D21</td>
            <td><strong>O₃_SCL</strong></td>
          </tr>
          <tr>
            <td> </td>
            <td><strong>WindSpeed</strong></td>
          </tr>
          <tr>
            <td>Power Supply 12V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>Power Supply GND <br> Arduino Mega 2560 GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>A1</td>
            <td><strong>OUT</strong></td>
          </tr>
          <tr>
            <td> </td>
            <td><strong>WindDirection</strong></td>
          </tr>
          <tr>
            <td>5V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>A0</td>
            <td><strong>OUT</strong></td>
          </tr>
        </table>
      </td>
      <td>
        <table border="1">
          <tr>
            <th><strong>Arduino Mega 2560 (3개 동일)</strong></th>
            <th><strong>PM1, PM2, PM3</strong></th>
          </tr>
          <tr>
            <td>5V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>D19</td>
            <td><strong>PM1_Tx</strong></td>
          </tr>
          <tr>
            <td>D17</td>
            <td><strong>PM2_Tx</strong></td>
          </tr>
          <tr>
            <td>D15</td>
            <td><strong>PM3_Tx</strong></td>
          </tr>
          <tr>
            <td>Power Supply 5V</td>
            <td><strong>Fan1, Fan2, Fan3 VCC</strong></td>
          </tr>
          <tr>
            <td>Power Supply GND <br> Arduino Mega 2560 GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>D8</td>
            <td><strong>Fan1 PWM</strong></td>
          </tr>
          <tr>
            <td>D9</td>
            <td><strong>Fan2 PWM</strong></td>
          </tr>
          <tr>
            <td>D10</td>
            <td><strong>Fan3 PWM</strong></td>
          </tr>
          <tr>
            <td>D2</td>
            <td><strong>Fan1 Tach</strong></td>
          </tr>
          <tr>
            <td>D3</td>
            <td><strong>Fan2 Tach</strong></td>
          </tr>
          <tr>
            <td>D21</td>
            <td><strong>Fan3 Tach</strong></td>
          </tr>
        </table>
      </td>
    </tr>
  </table>
</div>


<br><br>

## Fan 파라미터 조정

<table border="1" align="center">
  <tr>
    <th>Container</th>
    <th>Module</th>
    <th>note</th>
  </tr>
  <tr>
    <td>Container 1</td>
    <td>M1_P1_? <br> M1_P2_? <br> M1_P3_?</td>
    <td rowspan="3">각 컨테이너의 각 모듈의 원하는 Fan 속도를 입력하면 그 Fan 속도로 Fan을 움직임 <br> (?: Fan RPM 값, Mx_Py : x는 각 컨테이너를 가리키고, y는 각 모듈을 가리킴)</td>
  </tr>
  <tr>
    <td>Container 2</td>
    <td>M2_P1_? <br> M2_P2_? <br> M2_P3_?</td>
  </tr>
  <tr>
    <td>Container 3</td>
    <td>M3_P1_? <br> M3_P2_? <br> M3_P3_?</td>
  </tr>
</table>




<br><br>

## Server (OneM2M)

### Mobius 플랫폼
- URL: 필요시 제공해주겠음
- [Mobius 플랫폼 접속 링크](http://114.71.220.59:7575/#!/monitor)

![server](https://github.com/user-attachments/assets/3d25239d-c8fb-4218-8019-f742800bbce3)
