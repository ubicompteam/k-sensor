#include <Wire.h>
#include <DHTStable.h>              //DHT22

#define SLAVE_ADDRESS 0x5A

// Register addresses
#define WRITE_REGISTER 0x01
#define READ_REGISTER 0x02
#define START_SIGNAL 0xA0

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

//dht var
float temp, humi;

char CO_data[60];    //Hardware-designed co data archiving
char NO2_data[60];    //Hardware-designed no2 data archiving
char SO2_data[60];    //Hardware-designed so2 data archiving
float direct[] = {3.84,1.98,2.25,0.41,0.45,0.32,0.90,0.62,1.40,1.19,3.08,2.93,4.62,4.04,4.33,3.43};
float wind_speed_print_F = 0.0;
float degree_F = 0.0;

int SS_count = 0;

bool startReceived = false;
char con_data[64];

float value_convert(String value);
void WW();
bool runEvery(unsigned long interval);

void setup() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  delay(100);
  
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);

}

void loop() {
  /*
    if(runEvery(1000)){
      WW();
    }
    */
}

void receiveEvent(int howMany) {
    int registerAddress = Wire.read();
    Serial.println(registerAddress);
    
    if (registerAddress == READ_REGISTER && Wire.available() > 0) {
        Serial.println("readcome");
        int signal = Wire.read();
        Serial.println(signal);
        if (signal == START_SIGNAL) {
          startReceived = true;
        }
    }
}

void requestEvent() {
    if (startReceived) {
        Serial.println("come in");
        startReceived = false;
        //SO2, NO2, CO
        Serial1.write('\r');
        Serial2.write('\r');
        Serial3.write('\r');
        //delayMicroseconds(30);
    
        int i = 0;
    
        //SO2, NO2 data get
        while (Serial1.available()) {
          CO_data[i++] = Serial1.read();
        }
        //delayMicroseconds(10);
        i = 0;
        while (Serial2.available()) {
          NO2_data[i] = Serial2.read();
          i++;
        }
        //delayMicroseconds(10);
        i = 0;
        while (Serial3.available()) {
          SO2_data[i] = Serial3.read();
          i++;
        }
    
        String str_co_data = "";
        String str_no2_data = "";
        String str_so2_data = "";
    
        for(int j = 12; j < 18; j++){
          str_co_data += CO_data[j];
          str_no2_data += NO2_data[j];
          str_so2_data += SO2_data[j];
        }
    
        float coPPM, no2PPM, so2PPM;
    
        coPPM = value_convert(str_co_data);
        no2PPM = value_convert(str_no2_data);
        so2PPM = value_convert(str_so2_data);
        
        //0인 값 예외처리
        if (SS_count > 0) {
          if(wind_speed_print_F == 0.0){
            wind_speed_print_F = 0.0;
          }
          if(degree_F == 0.0){
            degree_F = 0.0;
          }
          if(wind_speed_print_F != 0.0 || degree_F != 0.0){
            degree_F = degree_F / (float)SS_count;
            wind_speed_print_F = wind_speed_print_F / (float)SS_count;
          }
        } else {
          degree_F = -1.0;
          wind_speed_print_F = -1.0;
        }
        
        //int ozoneConcentration = Ozone.readOzoneData(COLLECT_NUMBER);
    
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
        //float Ozone_ppm = (float)ozoneConcentration / 1000;
    
        snprintf(con_data, sizeof(con_data), "%d,%d,0,%03d,%03d,%03d,%d,%d", temp, humi, coPPM,no2PPM,so2PPM,degree_F,wind_speed_print_F);
        
        Wire.write((uint8_t*)con_data, strlen(con_data) + 1);
        
        Serial.println(con_data);
        
        degree_F = 0.0;
        wind_speed_print_F = 0.0;
        SS_count = 0;
        startReceived = false;
      }
}

float value_convert(String value){
  int start = value.indexOf(',') + 1;
  int end = value.indexOf(',', start);

  float con_ppm = 0.0;
  String con_value = value.substring(start, end);
  int convalue = con_value.toInt();
  con_ppm = (float)convalue / 1000;

  return con_ppm;
}

void WW(){
  int wind_speed_val = analogRead(A1);
  float wind_speed_outvoltage = wind_speed_val * (5.0 / 1023.0);
  float wind_speed_print = 6.0 * wind_speed_outvoltage;
  
  int wind_direction_val = analogRead(A0);
  float wind_direction_outvoltage = wind_direction_val * 5 / 1023.0;

  float degree;
  for(int j = 0; j < 16; j++){
    if(direct[j]-0.02 < wind_direction_outvoltage && direct[j]+0.02 > wind_direction_outvoltage){
      degree = j * 22.5;       
      break;
    }
  }

  wind_speed_print_F += wind_speed_print;
  degree_F += degree;
  SS_count++;
}

bool runEvery(unsigned long interval){
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
