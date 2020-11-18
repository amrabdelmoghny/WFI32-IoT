# Secure Cloud Connectivity and Voice Control Demo for Microchip WFI32-IoT Board .

Devices: **| PIC32 WFI32E | WFI32 | Trust\&Go (ECC608) |**

Features: **| Secure Cloud Connectivity | Voice Control |**

[![Latest release](https://img.shields.io/github/v/release/MicrochipTech/PIC32MZW1_Curiosity_OOB?include_prereleases&sort=semver&style=for-the-badge)](https://github.com/MicrochipTech/PIC32MZW1_Curiosity_OOB/releases/latest)
[![Latest release date](https://img.shields.io/github/release-date/MicrochipTech/PIC32MZW1_Curiosity_OOB?style=for-the-badge)](https://github.com/MicrochipTech/PIC32MZW1_Curiosity_OOB/releases/latest)
 

## Preface
The WFI32-IoT board is a compact, easy-to-use development board that supports rapid prototyping of IoT devices and demonstrates cloud connectivity with voice control enablement. This kit is also a useful tool to evaluate the features of WFI32E01PC, the single-chip Wi-Fi module.

The WFI32-IoT board contains the PIC32MZW1, a highly integrated IoT system core supporting smart Wi-Fi functionalities and a premium MCU. There are various types of on-board sensors for users to build up their applications. Users can also expand its functionality through MikroElectronika mikroBUS™ Click™ adapter boards. The on-board hardware crypto device can be used to make a secure and authenticated cloud connection.

The WFI32-IoT board includes an integrated programmer/debugger using PICkit™ On Board (PKOB4) and requires only a micro USB power cable to power up and program the board and evaluate the module.

The WFI32-IoT board features the following elements:
* WFI32E01PC, PIC32MZ W1 Wi-Fi Module with PCB antenna with built-in ATECC608A Trust and Go. 
* mikroBUS™ socket to expand functionality using MikroElectronika Click™ adapter boards.
* PICkit™ On Board (PKOB4) Universal Serial Bus (USB) Micro-B Socket for programming/debugging.
* 4 on-board LEDs and 2 user configurable buttons.
* On-board temperature sensor and light sensor.
* External flash 32Mb.
* Module power measurement point to test:
	* Power consumption of WFI32E01PC module only.
	* Power consumption of the whole board.
* 5 I/O test pads.
* USB Power charging circuit for Lithium-ion battery.

The WFI32-IoT board comes pre-programmed and configured for demonstrating the connectivity to the AWS Cloud IoT Core.

<p align="center">
<img src="resources/media/figure_preface_1.png"/>
</p>
 
## Getting Started

Want to quickly evaluate the WFI32-IoT board? Check out our **[Quick Start Guide]**(https://github.com/amrabdelmoghny/WFI32-IoT/blob/main/GettingStarted.md) to know how to connect out-of-the-box to Microchip AWS Sandbox account to visualize board's Temperature and Light sensors data and control on-board LED using just a web intrerface or even using Amazon Alexa Voice control!

---

## Application Deep Dive

Want to dive behind the scenes and learn how Connectivity, Security and Cloud are tied together? Check out out our **[Deep Dive Guide]**(https://github.com/amrabdelmoghny/WFI32-IoT/blob/main/HowItWorks.md) to know how easy it is to migrate the demo to your own cloud instance.
