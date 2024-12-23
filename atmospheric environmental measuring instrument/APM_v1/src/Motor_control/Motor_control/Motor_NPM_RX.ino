//받는 쪽
#include <PMS.h>

int d0;
bool startReceived = false;
String save = "";
String Send;

PMS pms(Serial2);
PMS::DATA data;

void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);

  pms.passiveMode();
}

void loop(){
  while(!startReceived){
    if(Serial1.available() > 0){
      save = Serial1.readStringUntil('\n');
      Serial1.read();
      Serial.println("come");
    }
    if(Serial.available() > 0){
      String Input = Serial.readStringUntil('\n');
      if(Input == "start"){
        startReceived = true;
        Serial.println("come2");
      }
    }
  }
  if(startReceived == true){
    pms.requestRead();
    if(pms.readUntil(data)){
      d0 = data.PM_AE_UG_2_5;
      Send = String(d0) + "," + save;
      Serial.println("come3");
      Serial.println(Send);

      startReceived = false;
    }
  }
}
