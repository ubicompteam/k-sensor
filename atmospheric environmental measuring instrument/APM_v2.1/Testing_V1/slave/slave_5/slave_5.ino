#include <PMS.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x5B

// Register addresses
#define WRITE_REGISTER 0x01
#define READ_REGISTER 0x02
#define START_SIGNAL 0xA0
#define GET_FEEDBACK 0xA1

int d0;
bool startReceived = false;
char con_data[64];

PMS pms(Serial1);
PMS::DATA data;

unsigned long previousMillis = 0;
const long interval = 10;  // 10ms 간격

void setup(){
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600);
  Serial1.begin(9600);

  pms.passiveMode();
}

void loop(){
  
}

void receiveEvent(int howMany) {
    int registerAddress = Wire.read();
    
    if (registerAddress == READ_REGISTER && Wire.available() > 0) {
        int signal = Wire.read();
        Serial.println(signal);
        if (signal == START_SIGNAL) {
          startReceived = true;
        }
    }
}

void requestEvent() {
    // millis()를 사용하여 타이밍을 처리
    unsigned long currentMillis = millis();
    if (startReceived && (currentMillis - previousMillis >= interval)) {
        previousMillis = currentMillis;  // 마지막 타이밍 업데이트

        startReceived = false;

        pms.requestRead();
        
        if (pms.readUntil(data)) {
          d0 = data.PM_AE_UG_2_5;
        }

        snprintf(con_data, sizeof(con_data), "%d", d0);
        
        Wire.write((uint8_t*)con_data, strlen(con_data) + 1);
        
        Serial.println(con_data);
    }
}
