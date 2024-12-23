# Sensor

## Language Selection
[English](README.md) | [Korean](README_KR.md)

<br><br>

## Code Description

- This code controls the fan speed using a 4-pin fan and calculates the fan speed in RPM.
- To adjust the fan speed, input the command in the format `M1_P1_fanSpeed` to the server's MQTT-registered motor topic.
- Fan RPM values are often unstable, especially when controlling three fans with a single Arduino Mega, leading to significant deviations.
   - These deviations are corrected using real-time proportional (P) control.

<br><br>

## Key Code Details

### Variable Description

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

- `pwmPins`: PWM pin numbers for the fans.
- `tachPins`: TACH pin numbers for the fans.
- `fanCount`: The number of fans being used.
- `tachCounters`: Stores pulse counts for each fan.
- `targetRPMs`: Stores the target RPM values from the input commands.
- `currentPWMs`: Stores the current PWM values to control fan speed.
- `targetReached`: Boolean array to track whether each fan has reached the target RPM.
- `minPWM`: Minimum PWM value to prevent negative values.
- `startReceived`: Used to signal when data extraction starts.
- `d1, d2, d3`: Variables to store particulate matter data from sensors.

<br>

### Command Processing Section

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

- When receiving an input starting with `M1_P`, the fan number and target RPM are extracted to control the specified fan.
  - For example, `M1_P1_1200` selects Fan 1 and sets its RPM target to 1200.
- When `start` is received, `startReceived` is set to `true`, triggering data extraction.

<br>

### Data Retrieval Section

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


- If `startReceived` is `true`, the code collects particulate matter data from each sensor and outputs the current RPM values for each fan.

<br>

### PWM Control Based on Target RPM (P-Control)

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

- Calculates the current RPM based on pulses and compares it with the target RPM to determine the `error` value.
- Adjusts PWM in increments based on the error, constraining it between `minPWM` and 255.
- When the RPM is within ±30 of the target, feedback is sent to the server.

<br>

### Fan Debugging

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

- Debugging can be enabled to print the current fan state, allowing you to monitor speed and pulse values when issues arise.
