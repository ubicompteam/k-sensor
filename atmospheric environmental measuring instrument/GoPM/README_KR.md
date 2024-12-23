# GoPM

## 언어 선택

[English](README.md) | [한국어](README_KR.md)

<br><br>

## 간략 설명
- 각각 팬 3개를 제어하여 3개의 미세먼지 센서의 공기 유입량을 조절하는 기구임
- 여기서 주로 다루는 건 코드 해석 및 하드웨어적으로 어떻게 구성되어있는지를 서술함

- 이 기구는 잿슨나노B01 4GB 제품과 아두이노 메가 2560을 사용하며,
- 우분투 20.04를 이용하고 nodejs 18버전을 사용중에 있음
- [우분투 20.04 링크](https://github.com/Qengineering/Jetson-Nano-Ubuntu-20-image)

<br><br>

## 젯슨나노가 꺼졌을 때 부터 기구를 작동하기 위한 간편 방법
1. 젯슨나노를 부팅
2. 아두이노 메가와 젯슨나노와 USB연결
3. 아두이노IDE를 실행에 아두이노에 `Sensor.ino` 업로드
4. nCube-Thyme-Nodejs 디렉토리에 들어감
  - ex) cd gw/nCube-Thyme-Nodejs
5. thyme.js가 있는 폴더에서 `node thyme` 입력후 엔터    
6. 이후 DAQ.py(finish.py) 실행
   - ex) cd gw
7. 마지막으로 app.js가 있는 곳에서 `node app.js` 실행
   - ex) cd gw/nCube-Thyme-Nodejs/tas_sample/tas_ledm

<br><br>

## 폴더 구조

```
GoPM/
├── src/
│   ├── DAQ/
│   │   └── DAQ.py
│   │
│   ├── Mobius_server_mqtt/
│   │   ├── nCube-Thyme-Nodejs.zip
│   │   ├── thyme/
│   │   │   └──  conf.js
│   │   └── tas/
│   │       ├── app.js
│   │       └── conf.xml
│   │   
│   ├── Sensor/
│   │   └── Sensor.ino
│  
├── HW/
│    └── README.md  


```

<br><br>

## 파일 설명

- src : 소스 코드들의 설명과 소스 코드들이 담긴 디렉토리
- HW : 제품의 하드웨어적인 구성도가 저장된 디렉토리
