// 보내는쪽
#include "m_3_1.h"

m_3_1 bottom(2, 3, 4, 28);
m_3_1 top_1(5, 6, 7, 26);
m_3_1 top_2(8, 9, 10, 22);
m_3_1 top_3(11, 12, 13, 24);

String pm1_save= "3";
String pm2_save= "3";
String pm3_save= "3";
String bottom_save = "0";
String con_data = "";

void setup(){
  Serial.begin(9600);
  Serial2.begin(9600);

  bottom.setRound(7200);
  top_1.setRound(56000);
  top_2.setRound(56000);
  top_3.setRound(56000);

  //10ms down
  bottom.setspeed(1000);
  top_1.setspeed(300);
  top_2.setspeed(300);
  top_3.setspeed(300);

  //bottom motor setup
  bottom.BottomSetup();

  //top motor setup
  top_1.TopSetup();
  top_2.TopSetup();
  top_3.TopSetup();

  bottom.stopMotor();
  top_1.stopMotor();
  top_2.stopMotor();
  top_3.stopMotor();
}

void loop() {
  if (Serial.available()) {
    // 문자열을 읽어서 처리
    String receivedString = Serial.readStringUntil('\n'); // 개행 문자까지 읽기
    
    //pm1, bottom 모터를 제어할 때 사용
    //mqtt로 제어를 받으면 해당되는 모터가 점검 즉 위치를 초기화할 때 다른 모터들도 멈추고, 이후 모든 모터들이 다시 움직임
    if (receivedString.startsWith("pm1")) {
      top_1.startMotor();
      top_1.TopSetup();
      int num = receivedString.substring(4).toInt(); // 문자열에서 숫자 부분 추출
      pm1_save = String(num);
      con_data = bottom_save + "," + pm1_save + "," + pm2_save + "," + pm3_save;
      top_1.TopLocation(num);
      Serial2.print(con_data);
    }else if(receivedString.startsWith("pm2")){
      top_2.startMotor();
      top_2.TopSetup();
      int num = receivedString.substring(4).toInt(); // 문자열에서 숫자 부분 추출
      pm2_save = String(num);
      con_data = bottom_save + "," + pm1_save + "," + pm2_save + "," + pm3_save;
      top_2.TopLocation(num);
      Serial2.print(con_data);   
    }else if(receivedString.startsWith("pm3")){
      top_3.startMotor();
      top_3.TopSetup();
      int num = receivedString.substring(4).toInt(); // 문자열에서 숫자 부분 추출
      pm3_save = String(num);
      con_data = bottom_save + "," + pm1_save + "," + pm2_save + "," + pm3_save;
      top_3.TopLocation(num);
      Serial2.print(con_data);     
    }else if (receivedString.startsWith("bottom")){
      bottom.startMotor();
      bottom.BottomSetup();
      int num = receivedString.substring(7).toInt();
      bottom_save = String(num);
      con_data = bottom_save + "," + pm1_save + "," + pm2_save + "," + pm3_save;      
      bottom.BottomLocation(num);
      Serial2.print(con_data);
    }
  }
  
  bottom.moveto();
  top_1.moveto();
  top_2.moveto();
  top_3.moveto();
}
