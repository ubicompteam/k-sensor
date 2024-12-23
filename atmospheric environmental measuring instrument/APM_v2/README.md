# APM_v2

## Language Selection

[English](README.md) | [한국어](README_KR.md)

<br><br>

## Overview

- The air pollution collection device is designed to achieve similar performance to high-cost particulate matter measurement equipment using low-cost particulate matter sensors.
- Based on seasonal correlation analysis of meteorological and air pollution data with high concentrations of particulate matter, sensors for particulate matter, temperature and humidity, sulfur dioxide, carbon monoxide, nitrogen dioxide, wind direction, and wind speed are installed.
- The device is positioned with sensors at a height of over 1.5 meters according to the Ministry of Environment's air pollution measurement and operation guidelines.
- Low-cost particulate matter sensors and environmental sensors (temperature and humidity, sulfur dioxide, carbon monoxide, nitrogen dioxide, wind direction, wind speed, ozone) are installed on the upper part of the device.
- After removing the fan inside the particulate matter sensor, a PWM-controlled fan is installed to allow control of air intake.

<br><br>

### Modeling and Installation Photos

<div align="center">

  | Modeling | Installation |
  |:---:|:---:|
  | <img src="https://github.com/user-attachments/assets/ba80964b-d8c8-4775-a51b-e386a5ecab33" width="470px" height="350px" alt="Modeling"> | <img src="https://github.com/user-attachments/assets/8cf09806-a0ee-473e-bbfc-5edc33816b9d" width="470px" height="350px" alt="Installation"> |

</div>

<br><br>

### APM_v2 Mqtt (Motor) Operation Video

- The motor was considered unnecessary and has been removed.

[View APM_v2 Mqtt Operation Video](https://youtu.be/hN8SpTdIn4Q?feature=shared)

<br><br>

### Directory Structure

```
APM_v2/
├── Document/
│   ├── Guide_KR
│   └── Guide_EN
│ 
├── src/
│   ├── DAQ/
│   │   └── DAQ.py
│   │
│   ├── Mobius_server_mqtt/
│   │   └── nCube-Thyme-Nodejs.zip
│   │
│   ├── Sensor/
│   │   ├── Environmental_Sensor/
│   │   │   ├── DFRobot_OzoneSensor.cpp
│   │   │   ├── DFRobot_OzoneSensor.h
│   │   │   └── Environmental_Sensor.ino
│   │   ├── Module_Floor_1/
│   │   │   └── Module_Floor_1.ino
│   │   ├── Module_Floor_2/
│   │   │   └── Module_Floor_2.ino
│   │   ├── Module_Floor_3/
│   │   │   └── Module_Floor_3.ino
│   │   ├── NPM
│   │   │   └── NPM.ino

```

<br><br>

### Hardware Equipment Used

| Equipment Name          | Specification                                                                                                                                                                                                                             | Quantity | Note                         |
|:-----------------------:|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:--------:|:----------------------------:|
| Jetson Nano B 01       | Quad-core ARM® Cortex-A57@ 1.43 GHz                                                                                                                                                                                                       | 1        |                              |
| Arduino Mega 2560      | ATmega2560                                                                                                                                                                                                                               | 4        | 3 for particulate matter data, 1 for environmental data |
| Arduino Uno            | ATmega328P                                                                                                                                                                                                                               | 1        |                              |
| PMS7003                | Light scattering method <br> (fan presence: None)                                                                                                                                                                                         | 9        | Particulate matter sensor     |
| Environmental Sensor    | CO, NO2, SO2 each 1 (AllSensing AGSM series) <br> Ozone 1 (sen0321) - I2C communication <br> WindSpeed 1 (sen0170) - Analog communication <br> WindDirection 1 (WS5029) - Analog communication <br> Temperature and Humidity (DHT22) - Digital |         | UART, I2C, Digital, Analog   |
| Drive Unit Sensor       | Fan                                                                                                                                                                                                                                      | 9        | 4-pin Fan                     |
| Others                  | Power supply 1 (waterproof) <br> Electrical box 358 x 270 x 152 1 <br> Electrical box 500 x 400 x 160 1 <br> Profile 40 x 40 x 500 6 <br> Profile 40 x 40 x 250 16 <br> Profile 20 x 80 x 400 8 <br> Profile 20 x 40 x 400 11 <br> Profile 20 x 40 x 360 8 <br> Profile 20 x 20 x 400 8 <br> Profile 20 x 20 x 470 10 <br> SSEBL420 8 <br> 4-wire shielded cable (about 1.5M) 9 <br> 2-wire shielded cable (1.5M) 8 <br> 3D printed materials (PLA+ material) |         |                              |

<br><br>

### Wiring Diagram

<div align="center">

  | Sensor Wiring | 
  |:---:|
  | <img src="https://github.com/user-attachments/assets/c789b326-338c-4222-890a-6d37527918c3" width="600px" height="550px" alt="Sensor Wiring Diagram"> |

</div>

<br>

### Sensor Wiring Diagram

<div align="center">
  <table>
    <tr>
      <td>
        <table border="1">
          <tr>
            <th><strong>Arduino Mega 2560 <br> (Environmental Sensor)</strong></th>
            <th><strong>Temperature, CO, NO2, SO2, O₃</strong></th>
          </tr>
          <tr>
            <td>5V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>D6</td>
            <td><strong>Temperature_OUT</strong></td>
          </tr>
          <tr>
            <td>D19</td>
            <td><strong>CO_Tx</strong></td>
          </tr>
          <tr>
            <td>D18</td>
            <td><strong>CO_Rx</strong></td>
          </tr>
          <tr>
            <td>D17</td>
            <td><strong>NO2_Tx</strong></td>
          </tr>
          <tr>
            <td>D16</td>
            <td><strong>NO2_Rx</strong></td>
          </tr>
          <tr>
            <td>D15</td>
            <td><strong>SO2_Tx</strong></td>
          </tr>
          <tr>
            <td>D14</td>
            <td><strong>SO2_Rx</strong></td>
          </tr>
          <tr>
            <td>D20</td>
            <td><strong>O₃_SDA</strong></td>
          </tr>
          <tr>
            <td>D21</td>
            <td><strong>O₃_SCL</strong></td>
          </tr>
          <tr>
            <td> </td>
            <td><strong>WindSpeed</strong></td>
          </tr>
          <tr>
            <td>Power Supply 12V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>Power Supply GND <br> Arduino Mega 2560 GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>A1</td>
            <td><strong>OUT</strong></td>
          </tr>
          <tr>
            <td> </td>
            <td><strong>WindDirection</strong></td>
          </tr>
          <tr>
            <td>5V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>A0</td>
            <td><strong>OUT</strong></td>
          </tr>
        </table>
      </td>
      <td>
        <table border="1">
          <tr>
            <th><strong>Arduino Mega 2560 (3 units, same)</strong></th>
            <th><strong>PM1, PM2, PM3</strong></th>
          </tr>
          <tr>
            <td>5V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>D19</td>
            <td><strong>PM1_Tx</strong></td>
          </tr>
          <tr>
            <td>D17</td>
            <td><strong>PM2_Tx</strong></td>
          </tr>
          <tr>
            <td>D15</td>
            <td><strong>PM3_Tx</strong></td>
          </tr>
          <tr>
            <td>Power Supply 5V</td>
            <td><strong>Fan1, Fan2, Fan3 VCC</strong></td>
          </tr>
          <tr>
            <td>Power Supply GND <br> Arduino Mega 2560 GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>D8</td>
            <td><strong>Fan1 PWM</strong></td>
          </tr>
          <tr>
            <td>D9</td>
            <td><strong>Fan2 PWM</strong></td>
          </tr>
          <tr>
            <td>D10</td>
            <td><strong>Fan3 PWM</strong></td>
          </tr>
          <tr>
            <td>D2</td>
            <td><strong>Fan1 Tach</strong></td>
          </tr>
          <tr>
            <td>D3</td>
            <td><strong>Fan2 Tach</strong></td>
          </tr>
          <tr>
            <td>D21</td>
            <td><strong>Fan3 Tach</strong></td>
          </tr>
        </table>
      </td>
    </tr>
  </table>
</div>

<br><br>

## Fan Parameter Adjustment

<table border="1" align="center">
  <tr>
    <th>Container</th>
    <th>Module</th>
    <th>Note</th>
  </tr>
  <tr>
    <td>Container 1</td>
    <td>M1_P1_? <br> M1_P2_? <br> M1_P3_?</td>
    <td rowspan="3">Enter the desired Fan RPM for each module of each container. The fan will operate at that RPM <br> (?: Fan RPM value, Mx_Py : x refers to the container and y refers to the module)</td>
  </tr>
  <tr>
    <td>Container 2</td>
    <td>M2_P1_? <br> M2_P2_? <br> M2_P3_?</td>
  </tr>
  <tr>
    <td>Container 3</td>
    <td>M3_P1_? <br> M3_P2_? <br> M3_P3_?</td>
  </tr>
</table>

<br><br>

## Server (OneM2M)

### Mobius Platform
- URL: Will be provided if necessary
- [Mobius Platform Access Link](http://114.71.220.59:7575/#!/monitor)

![server](https://github.com/user-attachments/assets/3d25239d-c8fb-4218-8019-f742800bbce3)



