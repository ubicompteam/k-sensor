#include <PMS.h>                //pms7003
#include "DHTStable.h"              //DHT22
#include "DFRobot_OzoneSensor.h"
#include "RTClib.h"

RTC_DS3231 rtc;

DHTStable DHT;
#define DHT22_PIN       6
struct
{
    uint32_t total;
    uint32_t ok;
    uint32_t crc_error;
    uint32_t time_out;
    uint32_t connect;
    uint32_t ack_l;
    uint32_t ack_h;
    uint32_t unknown;
} counter = { 0,0,0,0,0,0,0,0};

#define COLLECT_NUMBER   20              // collect number, the collection range is 1-100
#define Ozone_IICAddress OZONE_ADDRESS_3

DFRobot_OzoneSensor Ozone;

PMS pms1(Serial3);
PMS::DATA data1;

PMS pms2(Serial2);
PMS::DATA data2;

PMS pms3(Serial1);
PMS::DATA data3;

uint16_t d1, d2, d3;
//dht var
float temp, humi;

bool startReceived = false;

unsigned long previousMillis = 0;
const unsigned long interval = 300000;  // 5분 간격


void setup() {
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  Serial.begin(9600);
  delay(1000);

  Ozone.begin(Ozone_IICAddress);
  Ozone.setModes(MEASURE_MODE_PASSIVE);

  pms1.passiveMode();
  pms2.passiveMode();
  pms3.passiveMode();

  //Serial.print("DataType: ");
  //Serial.println("NPMDust Dust1 Dust2 Dust3 temp humi Ozone");
}

void loop() {
  //젯슨에서 값 받는거 대기
  while (!startReceived) {
  // start 신호 받기
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      if (input == "start") {
        startReceived = true;
      }
    }
  }
  if (startReceived == true) {
    
    int ozoneConcentration = Ozone.readOzoneData(COLLECT_NUMBER);

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

   uint32_t start = micros();
    int chk = DHT.read22(DHT22_PIN);
    uint32_t stop = micros();

    counter.total++;
    switch (chk)
    {
    case DHTLIB_OK:
        counter.ok++;
        //Serial.print("OK,\t");
        break;
    case DHTLIB_ERROR_CHECKSUM:
        counter.crc_error++;
        //Serial.print("Checksum error,\t");
        break;
    case DHTLIB_ERROR_TIMEOUT:
        counter.time_out++;
        //Serial.print("Time out error,\t");
        break;
    default:
        counter.unknown++;
        //Serial.print("Unknown error,\t");
        break;
    }
    humi = DHT.getHumidity();
    temp = DHT.getTemperature();
    float Ozone_ppm = (float)ozoneConcentration / 1000;

    Serial.print(d1);
    Serial.print(",");
    Serial.print(d2);
    Serial.print(",");
    Serial.print(d3);
    Serial.print(",");
    Serial.print(temp);
    Serial.print(",");
    Serial.print(humi);
    Serial.print(",");
    Serial.println(Ozone_ppm, 3);
    startReceived = false;
  }
}
