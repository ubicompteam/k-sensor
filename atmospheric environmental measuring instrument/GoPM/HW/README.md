# HW

## Language Selection

[English](README.md) | [한국어](README_KR.md)

<br><br>

## Mechanical Structure

- **Control Box**: Waterproof box containing the Jetson Nano and Arduino Mega 2560.
- **Power Box**: Waterproof box with a power supply and DC converter.
- **Module**: Contains fine dust sensors and fans.
- **Container**: Holds a total of three modules.

<br><br>

## Photos
<div align="center">

  | Control Box | Power Box |
  |:---:|:---:|
  | <img src="https://github.com/user-attachments/assets/56330ad7-43f5-4b0f-bd80-c847cda8fec0" width="470px" height="350px" alt="Control Box"> | <img src="https://github.com/user-attachments/assets/7fbeb9d5-a5bd-4872-bfaa-b6e28adff76e" width="470px" height="350px" alt="Power Box"> |

  <br>

  | Three Modules | Module Wiring |
  |:---:|:---:|
  | <img src="https://github.com/user-attachments/assets/05dba7e3-ae4c-4019-b996-64883652fd07" width="470px" height="350px" alt="Three Modules"> | <img src="https://github.com/user-attachments/assets/479d286e-7784-4e44-86e6-6b0261eed227" width="470px" height="350px" alt="Module Wiring"> |

</div>

<br><br>

## Photo Descriptions

### Control Box Description

- The Jetson Nano and Arduino Mega are located here and communicate via serial to exchange data.
- The wires entering from below are connected to the three modules within the container.
- The DC converter above the Jetson Nano steps down 12V from the Power Box to 5V to power the fan externally.

### Power Box Description

- The Power Box houses two waterproof power supplies, with one connected to the GoPM.
- The power supply is set to 12V to power an anemometer (another device), which then further steps down in the Control Box.

### Module Description

- The modules are arranged at 120-degree angles and held by magnets for easy maintenance inside the container.
- Measured with a tachometer, it was confirmed that the error range between the fan's RPM and the tachometer's RPM differed by about 15.
- Inside each module, a PMS7003 sensor (with fan removed) and an installed fan are set up with a duct, allowing the fan to control air intake for the sensor.
- Vent holes are added to the modules to ensure proper air circulation.

### Module Wiring Description

- Close-up photo of the wiring attached to each module.
- There is a notch between pins 7 and 1 (the orientation should be aligned based on this notch).
- Shielded cables are used to minimize noise interference, as tachometer wires are noise-sensitive.

  | Module Wiring |
  |:---:|
  | <img src="https://github.com/user-attachments/assets/2c8c64b9-2896-46e6-81fb-831fb3ab206a" width="470px" height="350px" alt="Module Wiring"> |
