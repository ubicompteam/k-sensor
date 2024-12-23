#include <Wire.h>

#define SLAVE_ADDRESS 0x5A

// Register addresses
#define WRITE_REGISTER 0x01
#define READ_REGISTER 0x02
#define START_SIGNAL 0xA0


bool startReceived = false;
char con_data[32];

void setup() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  delay(100);
  
  Serial.begin(9600);

}

void loop() {
}

void receiveEvent(int howMany) {
    int registerAddress = Wire.read();
    
    if (registerAddress == READ_REGISTER && Wire.available() > 0) {
        int signal = Wire.read();
        if (signal == START_SIGNAL) {
          startReceived = true;
        }
    }
}

void requestEvent() {
    if (startReceived) {
        startReceived = false;
    
        // Write a string to con_data and ensure it is null-terminated
        snprintf(con_data, sizeof(con_data), "1,2,3,4,5,6,7,8");
        
        // Ensure that the string ends with '\0'
        con_data[sizeof(con_data) - 1] = '\0';
        
        // Send the data to the master (Jetson Nano) over I2C
        Wire.write((uint8_t*)con_data, strlen(con_data) + 1);
        
        // Print the data to Serial monitor for debugging
        Serial.println(con_data);
    }
}
