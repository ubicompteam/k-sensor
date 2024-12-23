# APM_v1

## Language Selection

[English](README.md) | [한국어](README_KR.md)

<br><br>

## Overview

- The air pollution collection device is designed to achieve performance similar to high-cost particulate matter measurement equipment by using low-cost particulate matter sensors.
- Based on seasonal correlation analysis studies of weather, air pollutants, and high-concentration particulate matter, sensors for particulate matter, temperature and humidity, sulfur dioxide, carbon monoxide, nitrogen dioxide, wind direction, and wind speed are installed.
- The device positions sensors at a height of more than 1.5 meters according to the Ministry of Environment’s air pollution measurement and operation guidelines.
- On the top of the device, low-cost particulate matter measurement sensors and environmental sensors (temperature and humidity, sulfur dioxide, carbon monoxide, nitrogen dioxide, wind direction, wind speed, ozone) are installed.
- The particulate matter sensors are designed to be adjustable vertically and rotate 360 degrees, allowing users to adjust the sensor's position according to their commands.

<br><br>

### Modeling and Installation Photos
<div align="center">
  
  | Modeling | Installation |
  |:---:|:---:|
  | <img src="https://github.com/user-attachments/assets/16247e7f-1541-47be-a514-2af5a8bb7449" width="420px" height="300px" alt="Modeling"> | <img src="https://github.com/user-attachments/assets/54e6a5d6-ec62-4971-b192-cef102818fc4" width="420px" height="300px" alt="Installation"> |
</div>

<br><br>

### APM_v1 MQTT Operation Video

[Watch APM_v1 MQTT Operation Video](https://youtube.com/shorts/-vXDwXWd0H0?feature=share)

<br><br>

### Wiring Diagram

<div align="center">
  
  | Sensor Wiring | Motor Wiring |
  |:---:|:---:|
  | <img src="https://github.com/user-attachments/assets/b9ae9f78-adc2-4b7d-a810-ac639ee7c0d8" width="420px" height="300px" alt="Sensor Wiring Diagram"> | <img src="https://github.com/user-attachments/assets/166d0f0b-ee65-4d67-88f7-16cb4cec47f2" width="420px" height="300px" alt="Motor Wiring Diagram"> |
</div>

<br>

### Directory Structure

```
APM_v1/
├── Document/
│   ├── APM_1_v1.0.docx
│   └── APM_1_v1.0.pdf
│
├── src/
│   ├── DAQ/
│   │   └── DAQ.py
│   │
│   ├── Mobius_server_mqtt/
│   │   ├── HowToUsing_nCube.zip
│   │   └── nCube-Thyme-Nodejs.zip
│   │
│   ├── Motor_control/
│   │   ├── Motor_auto_control/
│   │   │   └── auto_control.py
│   │   ├── Motor_control/
│   │   │   ├── Motor_NPM_RX.ino
│   │   │   └── Motor_NPM_TX.ino
│   │   ├── Motor_library/
│   │   │   ├── m_3_1.cpp
│   │   │   └── m_3_1.h
│   │   └── README.md
│   │
│   ├── Sensor/
│       ├── CNSWW_new.ino
│       └── DDDOT.ino

```

<br><br>

### Used Hardware Equipment

| Equipment Name        | Specifications                                                                                                                                                                                                                                                                              | Quantity | Notes                           |
|:---------------------:|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:--------:|:-------------------------------:|
| Jetson Nano B 01     | Quad-core ARM® Cortex-A57@ 1.43 GHz                                                                                                                                                                                                                                                           | 1        |                                |
| Arduino Mega 2560    | ATmega2560                                                                                                                                                                                                                                                                                    | 3        | 1 for motor control, 2 for data transmission |
| Arduino Uno          | ATmega328P                                                                                                                                                                                                                                                                                    | 1        |                                |
| PMS7003              | Optical scattering method                                                                                                                                                                                                                                                                     | 4        | Particulate matter sensor       |
| Environmental Sensors | CO, NO2, SO2 each 1 (AllSensing AGSM series) <br> Ozone 1 (sen0321) - I2C communication <br> WindSpeed 1 (sen0170) - Analog communication <br> WindDirection 1 (WS5029) - Analog communication <br> Temperature and Humidity (DHT22) - Digital |          | UART, I2C, Digital, Analog     |
| Actuator Sensors      | Vertical initialization limit switches 3 <br> Horizontal initialization switch 1 <br> Cooler control toggle switch 1 <br> On/Off switches 2 <br> Stepper motors NEMA17 6 (Torque: 4.0 3, 6.5 3)                                                                                             |          |                                |
| Others                | Profile 30 x 30 20 pieces <br> Profile 20 x 20 4 pieces <br> Bearings 6 <br> Small bearing 1 (for horizontal initialization support) <br> Round rod 1 <br> Outsourced acrylic plates 6 <br> Power supplies 2 (LRS-350-24) <br> Remaining materials made with 3D printer (PLA+ material) |          |                                |


<br><br>

#### Sensor Wiring Diagram

<div align="center">
  <table>
    <tr>
      <td>
        <table>
          <tr>
            <th><strong>Arduino Mega 2560 1</strong></th>
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
            <td><strong>PM3_Dx</strong></td>
          </tr>
          <tr>
            <td></td>
            <td><strong>DHT22</strong></td>
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
            <td><strong>OUT</strong></td>
          </tr>
          <tr>
            <td></td>
            <td><strong>Ozone</strong></td>
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
            <td>D20</td>
            <td><strong>SDA</strong></td>
          </tr>
          <tr>
            <td>D21</td>
            <td><strong>SCL</strong></td>
          </tr>
        </table>
      </td>
      <td>
        <table>
          <tr>
            <th><strong>Arduino Mega 2560 2</strong></th>
            <th><strong>CO, NO2, SO2</strong></th>
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
            <td></td>
            <td><strong>Wind Direction</strong></td>
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
          <tr>
            <td></td>
            <td><strong>Wind Speed</strong></td>
          </tr>
          <tr>
            <td>Power Supply 24V</td>
            <td><strong>VCC</strong></td>
          </tr>
          <tr>
            <td>Power Supply GND</td>
            <td><strong>GND</strong></td>
          </tr>
          <tr>
            <td>A1</td>
            <td><strong>OUT</strong></td>
          </tr>
        </table>
      </td>
    </tr>
  </table>
</div>

<br><br>

## Motor Parameter Adjustment

### Bottom
| **Parameter**            | **Description**                                    |
|--------------------------|----------------------------------------------------|
| `bottom_?`               | Rotate to the specified angle when the desired angle is input |

### Top
| **Parameter**            | **Description**                                    |
|--------------------------|----------------------------------------------------|
| `pm?_x`                  | X-axis position of PM sensor (? : PM sensor ID, X : 1, 2, 3) |
| `pm1_x`                  | X-axis position of PM1 sensor (1: Top, 2: Middle, 3: Bottom) |
| `pm2_x`                  | X-axis position of PM2 sensor (1: Top, 2: Middle, 3: Bottom) |
| `pm3_x`                  | X-axis position of PM3 sensor (1: Top, 2: Middle, 3: Bottom) |

<br><br>

## Server (OneM2M)

### Mobius Platform

- **Currently, the server is inaccessible due to the removal of APM_v1**

![server](https://github.com/user-attachments/assets/9e5f2f4d-9210-41cc-a25c-f3306b2330c8)

- [Access Mobius Platform](http://114.71.220.59:2021/Mobius/Ksensor_ubicomp)
