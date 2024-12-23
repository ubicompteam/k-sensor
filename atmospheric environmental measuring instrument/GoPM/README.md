# GoPM

## Language Selection

[English](README.md) | [한국어](README_KR.md)

<br><br>

## Overview
- This mechanism controls airflow for three dust sensors by managing three individual fans.
- The focus here is on code interpretation and detailing the hardware configuration.

- The setup includes a Jetson Nano B01 4GB and an Arduino Mega 2560, using Ubuntu 20.04 and Node.js version 18.
- [Ubuntu 20.04 Image Link](https://github.com/Qengineering/Jetson-Nano-Ubuntu-20-image)

<br><br>

## Quick Start Guide for Operating the Mechanism After Jetson Nano Shutdown
1. Boot up the Jetson Nano.
2. Connect the Arduino Mega to the Jetson Nano via USB.
3. Open the Arduino IDE and upload `Sensor.ino` to the Arduino.
4. Navigate to the `nCube-Thyme-Nodejs` directory.
   - Example: `cd gw/nCube-Thyme-Nodejs`
5. In the directory with `thyme.js`, enter `node thyme` and press Enter.
6. Next, run `DAQ.py` (or `finish.py` if specified).
   - Example: `cd gw`
7. Finally, navigate to the folder containing `app.js` and run `node app.js`.
   - Example: `cd gw/nCube-Thyme-Nodejs/tas_sample/tas_ledm`

<br><br>

## Directory Structure

```
GoPM/
├── src/
│   ├── DAQ/
│   │   └── DAQ.py
│   │
│   ├── Mobius_server_mqtt/
│   │   ├── nCube-Thyme-Nodejs.zip
│   │   ├── thyme/
│   │   │   └──  conf.js
│   │   └── tas/
│   │       ├── app.js
│   │       └── conf.xml
│   │   
│   ├── Sensor/
│   │   └── Sensor.ino
│  
├── HW/
│    └── README.md  

```

<br><br>

## File Descriptions

- **src**: Directory containing source code and descriptions of each source file.
- **HW**: Directory where the product's hardware configuration files are stored.


