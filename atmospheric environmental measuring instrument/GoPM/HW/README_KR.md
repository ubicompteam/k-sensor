# HW

## 언어 선택

[English](README.md) | [한국어](README_KR.md)

<br><br>

## 기구 구조

- 컨트롤박스 (젯슨나노와 아두이노 메가 2560이 들어간 방수박스)
- 전원 박스 (파워서플라이와 DC컨버터가 들어간 방수 박스)
- 미세먼지와 팬이 들어간 모듈
- 모듈이 총 3개가 들어간 컨테이너

<br><br>

## 사진
<div align="center">

  | 컨트롤박스 | 전원박스 |
  |:---:|:---:|
  | <img src="https://github.com/user-attachments/assets/56330ad7-43f5-4b0f-bd80-c847cda8fec0" width="470px" height="350px" alt="컨트롤박스"> | <img src="https://github.com/user-attachments/assets/7fbeb9d5-a5bd-4872-bfaa-b6e28adff76e" width="470px" height="350px" alt="전원박스"> |

  <br>

  | 모듈 3개 | 모듈배선 |
  |:---:|:---:|
  | <img src="https://github.com/user-attachments/assets/05dba7e3-ae4c-4019-b996-64883652fd07" width="470px" height="350px" alt="모듈 3개"> | <img src="https://github.com/user-attachments/assets/479d286e-7784-4e44-86e6-6b0261eed227" width="470px" height="350px" alt="모듈배선"> |

</div>

<br><br>

## 사진에 대한 설명

### 컨트롤박스에 대한 설명

- 젯슨나노와 아두이노 메가가 위치해 있고, 서로 시리얼통신을 활용하여 데이터 주고 받는중
- 밑에서 들어오는 전선들은 컨테이너안에 모듈 3개에 대한 전선들임
- 젯슨나노 위에 위치한 DC컨버터는 전원박스에서 들어오는 12V의 전압을 5V로 낮추어 주는 역할을 함 ( 외부 전압으로 FAN에 공급을 하기 위함 )

### 전원박스에 대한 설명

- 전원박스에는 방수 파워서플라이 2개가 있고, 그중 하나를 GoPM에 연결함
- 12V인 이유는 풍속계(다른 기구)에 12V를 인가해야하기 때문에 12V로 낮춘후, 컨트롤 박스에서 한번 더 낮추는 형식으로 함

### 모듈에 대한 설명

- 모듈은 자세히 찍진 않았지만 120도 각도로 총 3개가 컨테이너 안에 자석으로 고정되어 유지보수가 쉽게 하였음
- 타코미터로 측정하여 팬의 RPM과 타코미터의 RPM의 오차범위가 15정도 차이가 나는 것을 확인함
- 모듈안에는 FAN을 제거한 PMS7003과 FAN을 설치하고, 덕트로 PMS7003의 공기 유입량을 FAN으로 제어하게끔 설계함
- 모듈안에 배출 구멍을 뚫어 공기 순환이 잘되게 만듦

### 모듈 배선에 대한 설명

- 모듈에 붙어있는 부분에 대한 사진
- 7번과 1번 사이 홈이 있음 ( 기준은 홈으로 집아야함 )
- 선은 쉴드(차폐)선을 이용하여 노이즈를 최소화 시킴 ( tach선이 노이즈에 민감 )

  | 모듈배선 |
  |:---:|
  | <img src="https://github.com/user-attachments/assets/2c8c64b9-2896-46e6-81fb-831fb3ab206a" width="470px" height="350px" alt="모듈배선"> |

