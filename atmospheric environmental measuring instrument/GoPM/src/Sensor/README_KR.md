# Sensor

## 언어 선택

[English](README.md) | [한국어](README_KR.md)

<br><br>

## 코드 설명

- 해당 코드는 4Pin Fan을사용하여 Fan의 속도를 RPM으로 구하고, RPM을 들어오는 명령에 맞게 제어함
- 명령어는 `M1_P1_팬속도` 를 서버 mqtt가 등록된 motor에 입력하면 됨
- Fan의 RPM은 불안정하고, 3개를 한개의 아두이노 메가에서 제어하다 보니 오차가 많이 생김
   - 해당 오차는 실시간 P제어로 해결함
 
<br><br>

## 중요 코드 상세 설명

### 변수 설명

```
const int pwmPins[] = {8, 9, 10};    // 팬의 PWM 핀들
const int tachPins[] = {2, 3, 21};   // 팬의 TACH 핀들
const int fanCount = 3;              // 팬의 수

volatile int tachCounters[fanCount] = {0, 0, 0};
unsigned long previousMillis = 0;
const long interval = 1000;  // 1초마다 RPM 계산

int targetRPMs[fanCount] = {900, 900, 900}; // 목표 RPM
int currentPWMs[fanCount] = {30, 30, 30}; // 현재 PWM 값
bool targetReached[fanCount] = {true, true, true}; // 목표 RPM 도달 여부 추적

const int minPWM = 30; // 최소 PWM 값

bool startReceived = false;

uint16_t d1, d2, d3;

```

- pwmPins : 팬의 PWM 핀 번호
- tachPins : 팬의 TACH 핀 번호
- fanCount : 사용되는 fan 갯수
- tachCounters : 펄스값이 저장되는 변수
- targetRPMs : 명령어가 들어오면 명령어의 목표 RPM값을 저장하는 변수
- currentPWMs : 현재 PWM값을 저장하는 변수 ( Fan은 PWM으로 제어가 가능해서 필요함 )
- targetReached : 명령어로 들어온 목표 값에 도달했을 때 피드백을 주기위한 bool변수
- minPWM : pwm이 음수로 갈 경우를 방지하는 변수
- startReceived : 데이터를 뽑아 간다는 신호를 가려내기 위한 변수
- d1, d2, d3 : 각각의 미세먼지 데이터를 저장하는 변수

<br>

### 명령어 처리하는 부분

```
    // 새로운 입력 형식 M1_P 처리
    if (input.startsWith("M1_P")) {
      int fanIndex = input.substring(4, 5).toInt() - 1;  // 팬 인덱스 (0부터 시작)
      int fanSpeed = input.substring(6).toInt();  // 입력된 RPM 값
      if (fanIndex >= 0 && fanIndex < fanCount) {
        targetRPMs[fanIndex] = fanSpeed;
        targetReached[fanIndex] = false; // 새로운 목표 RPM이 설정되면 도달 플래그 초기화
      }
    } else if(input == "start") {
      startReceived = true;
    }

```

- M1_P라고 시작되는 값이 들어오면 거기서 제어할 fan 번호를 추출하고, 목표 fan 값을 추출하여 해당 fan을 제어
  - ex) `M1_P1_1200` P에서 1의 값 추출하여 1번 Fan이 선택되고, 1200을 추출하여 1번 Fan을 1200의 RPM으로 값을 바꿈
- start 값이 들어오면 데이터 추출하는 명령어임으로 startReceived값을 True로 변경함

<br>

### 데이터 가져오는 부분

```
  if (startReceived == true) {
    pms1.requestRead();
    pms2.requestRead();
    pms3.requestRead();
    
    if(pms1.readUntil(data1)){
      d1 = data1.PM_AE_UG_2_5;
    }

    if(pms2.readUntil(data2)){
      d2 = data2.PM_AE_UG_2_5;
    }
    
    if(pms3.readUntil(data3)){
      d3 = data3.PM_AE_UG_2_5;
    }

    for (int i = 0; i < fanCount; i++) {
      Serial.print(targetRPMs[i]);
      Serial.print(",");
    }
    Serial.print(d1);
    Serial.print(",");
    Serial.print(d2);
    Serial.print(",");
    Serial.print(d3);
    Serial.println();

    startReceived = false;
  }
```

- startReceived가 True가 되면 이 코드를 실행시킴
- 미세먼지 값을 전부 수집하고, 현재 Fan들의 RPM값을 출력함

<br>

### 목표 RPM값을 토대로 P제어하면서 PWM제어

```
// 1초 간격으로 RPM 계산 및 PWM 조정
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    for (int i = 0; i < fanCount; i++) {
      int realRPM = (tachCounters[i] * 60) / 2;
      tachCounters[i] = 0; // tachCounter 초기화

      // 목표 RPM에 맞게 PWM 조정
      int error = targetRPMs[i] - realRPM;
      int pwmStep = 0;

      // 오차가 30 이상일 때만 PWM을 조정
      if (abs(error) > 1000) {
        pwmStep = 20;
      } else if (abs(error) > 450) {
        pwmStep = 10;
      } else if (abs(error) > 200) {
        pwmStep = 5;
      } else if (abs(error) > 30) {
        pwmStep = 1;
      }

      // 오차에 따라 PWM 값을 증가 또는 감소
      if (pwmStep > 0) {
        currentPWMs[i] += (error > 0) ? pwmStep : -pwmStep;
        currentPWMs[i] = constrain(currentPWMs[i], minPWM, 255);  // PWM 값이 최소값과 255 범위 내에 있도록 제한
        analogWrite(pwmPins[i], currentPWMs[i]);
      }

      // 목표 RPM에 도달했는지 확인 (첫 번째로 도달한 경우만 finish 출력)
      if (abs(error) <= 30 && !targetReached[i]) {  // 오차가 ±30 이내일 때
        Serial.print("M1_P");
        Serial.print(i+1);
        Serial.print("_");
        Serial.print(realRPM);
        Serial.println();
        targetReached[i] = true; // 플래그 설정하여 이후 출력 방지
      }
```

- 들어오는 펄스를 RPM으로 변환하기 위해 60을 곱하고, 한펄스에 2번 값을 매기니 2를 나누어줌
- 현재 RPM과 목표 RPM을 비교하여 error값을 구함
- error값에 따라 pwm의 범위를 정함
- 정의된 pwm의 범위로 pwm을 증가 감소하며 목표 RPM을 유지함
- 그리고 목표 RPM에 도달하면 서버에 값을 줄 피드백을 만듦

<br>

### Fan 디버깅

```
      // 시리얼 모니터에 현재 상태 출력 (주석 처리된 부분)
      //Serial.print("Fan ");
      //Serial.print(i + 1);
      //Serial.print(" | Target RPM: ");
      //Serial.print(targetRPMs[i]);
      //Serial.print(" | Real RPM: ");
      //Serial.print(realRPM);
      //Serial.print(" | PWM: ");
      //Serial.println(currentPWMs[i]);

```

- Fan에 문제가 생길시 해당 디버깅으로 Fan의 속도, 펄스 값을 파악할 수 있음
