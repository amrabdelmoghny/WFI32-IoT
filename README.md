# Getting Started Guide: Secure Cloud Connectivity and Voice Control Demo for Microchip WFI32-IoT Board .

Devices: **| PIC32 WFI32E | WFI32 | Trust\&Go (ECC608) |**

Features: **| Secure Cloud connectivity | Voice Control |**

[**Frequently Asked Questions**](https://github.com/MicrochipTech/PIC32MZW1_Curiosity_OOB/blob/master/faq.md)

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

MPLAB® Harmony v3 Configurator with supporting WFI32-IoT library can be used for generating the firmware through MPLAB X®. To download/modify the firmware, the system requirements are the following:
* IDE	MPLAB X IDE v 5.40 or later
* Compiler	XC32 Compiler v 2.41 or later
* Code Generation	MCC Plug-in v 3.75
* MCC Foundation Services v 0.1.32
* MCC PIC-IoT WG Sensor Node v 1.1.1
 
## Table of Contents

1. [Chapter1: Overview](#chapter1)
	1. 	[1.1 Board Layout](#chapter1.1)
	2.	[1.2 LED Indicators](#chapter1.2)
	3.	[1.3 Switch Button Use Cases](#chapter1.3)
2. [Chapter 2: Getting Started](#chapter2)
	1.	[2.1 Connecting the Board to the Host PC](#chapter2.1)
	2.	[2.2 The MCHP-IoT Webpage](#chapter2.2)
	3.	[2.3 Connecting the Board to Wi-Fi Networks](#chapter2.3)
	4.	[2.4 Visualizing Cloud Data in Real Time](#chapter2.4)
	5.	[2.5 Voice Control](#chapter2.5)
	6.	[2.6 Configuring Other Settings](#chapter2.6)
3.	[Chapter 3: Switching Cloud Instances](#chapter3)
	1. 	[3.1 Connecting to your own cloud instance](#chapter3.1)
	2. 	[3.2 Restore factory default cloud instance](#chapter3.2)
4.	[Chapter 4: Application Overview](#chapter4)
	1. 	[4.1 Application Structure](#chapter4.1)
	2. 	[4.2 Cloud Interaction](#chapter4.2)
5.	[Chapter 3: Code Generation](#chapter5)
	1. 	[5.1 Generating code from MHC](#chapter5.1)
6.	[FAQs, Tips and Troubleshooting](#chapter6)
	1.	[6.1 FAQs and Tips](#chapter6.1)
	2.	[6.2 LED Status Troubleshooting](#chapter6.2)
7.	[Appendix](#chapter7)
	1. 	[7.1 Relevant Links](#chapter7.1)

 

## Chapter 1: Overview <a name="chapter1"></a>

### 1.1 Board Layout <a name="chapter1.1"></a>
The WFI32-IoT board layout can be seen below.

### 1.2 LED Indicators <a name="chapter1.2"></a>
The WFI32-IoT board features four LEDs that the demo code uses to provide diagnostic information as represented in the table below.
LED Color | Label | Pattern | Indication | Details
--- | --- | --- | --- | ---
Blue	| WIFI  | Solid Blue 	| Wi-Fi Network Connection | Indicates a successful connection to the local Wi-Fi network.
Blue	| WIFI | Blinking Blue | Soft AP Mode | Indicates that the board can be detected and used as a Wi-Fi access point. For details refer to the 2.3 section.
Green	| CONN 	| Solid Green	| AWS Cloud Connection | Indicates a successful connection to the AWS Cloud.
Green 	| CONN| Blinking Green | Default Wi-Fi credentials |	Indicates that the board is trying to establish a Wi-Fi connection using the default credentials.
Yellow	| DATA | Blinking Yellow | Data Publication to the Cloud | Indicates that sensor data in the form of MQTT packet has been successfully published to the AWS Cloud.
Yellow	| DATA | Solid 	Yellow for extended time | State of Toggle sent within MQTT publish packet | Indicates the state of the “Toggle” switch, received as part of the packet published by AWS cloud on the subscribed topic.
Red	| ERROR | Solid Red| Error Status | Indicates an error in the application.

### 1.3 Switch Button Use Cases <a name="chapter1.3"></a>
The WFI32-IoT board features two user buttons where holding them on power up can configure the board as represented in the table below.
SW0     | SW1  	| Action
 ------ | ----- | ------ 
Held  	| 	| Enter Soft AP mode
Held	| Held	| Use default Wi-Fi credentials {**MCHP.IOT, microchip**}
 

## Chapter 2: Getting Started <a name="chapter2"></a>

### 2.1 Connecting the Board to the Host PC <a name="chapter2.1"></a>
1. The WFI32-IoT can be connected to a computer using a standard micro-USB cable. 
2. Once plugged in, the LED array will blink in the following order twice: **Blue -> Green -> Yellow -> Red**. 
3. When the board is not connected to Wi-Fi, the red LED will light up.
4. The board will appear as a removable storage device on the host PC
5. Double-click on the **CLICK-ME.HTM** file to go to the demo webpage.

**Note**:  All procedures are the same for Windows®, Mac OS®, and Linux® environments.
<p align="center">
<img src="resources/media/figure_2_1_1.png"/>
</p>

The CURIOSITY drive should contain the following five files:
* **CLICK-ME.HTM** - redirects the user to the WFI32-IoT web demo application
* **VOICE.HTM** - redirects the user to the WFI32-IoT device registration page for Alexa voice support
* **KIT-INFO.HTM** - redirects the user to WFI32-IoT product page
* **WIFI.CFG** - a text file with Wi-Fi credentials (defaults are **SSID:MCHP.IOT, Passphrase:microchip, Security:WPA/WPA2**)
* **CLOUD.JSON** - a JSON file with cloud configuration: Endpoint (default is Microchip's Sandbox account) and Client ID.
 

### 2.2 The MCHP-IoT Webpage <a name="chapter2.2"></a>
The WFI32-IoT webpage is used to:
1. Display the sensors' data.
2. Control on board Yellow LED.
3. Generate the Wi-Fi credentials as a file labeled **WIFI.CFG**.

The status markers at the middle of the page, as shown in the following figure, indicate the progress of the system setup. Markers from right to left are:

1. The Presence marker indicates if the board is connected to the host PC. 
2. The Wi-Fi marker indicates if the board is connected to a Wi-Fi network.
3. The Cloud marker indicates if the board is connected to AWS cloud. 
4. The Data marker indicates data is being sent from the board to cloud.
<p align="center">
<img src="resources/media/figure_2_2_2.png"/>
</p>

### 2.3 Connecting the Board to Wi-Fi Networks <a name="chapter2.3"></a>

#### Via WFI32-IoT Webpage
 <p align="center">
<img src="resources/media/figure_2_3_1.png"/>
</p>

1. The lower left-hand corner of the web page will show a wireless network connection window.
2. Once the required details are entered, click the **Download Configuration** button. 
3. A file named **WIFI.CFG** (text) file is downloaded to the host PC. 
4. Drag and drop the file to the **CURIOSITY drive** to update the Wi-Fi credentials of the board.
5. Reboot the device.
6. The **blue LED** will light up once a successful connection to the Wi-Fi Access Point is made.

**Important**: The Wi-Fi network SSID and password are limited to 19 characters. Avoid using quotation marks, names, or phrases that begin or end in spaces. The WFI32-IoT board supports only 2.4 GHz networks inline, thus using mobile hotspots to connect the board to the Internet is recommended.

**Note**: Any information entered in the SSID and password fields is not transmitted over the web or to the Microchip or AWS servers. Instead, the information is used locally (within the browser) to generate the WIFI.CFG file.

 
#### Via Soft AP
1. Download **Microchip Wi-Fi provisioning Mobile application** for Android here and for iOS here to your Mobile phone or Tablet.
2. The Soft AP mode can be entered by pressing and holding the **SW0** push button for most of the power up time. 
3. The **blue LED** will blink when Soft AP is available.
4. The board can be detected as a Wi-Fi access point named **WFI32-IoT_<MAC_ADDRESS>**. 
5. Using the Mobile phone or tablet, connect to the **WFI32-IoT_<MAC_ADDRESS>** hotspot. 

<img src="resources/media/figure_2_3_3.png" width="240"/>

6. List of available APs is shown and you can press **SCAN** button to refresh.
<img src="resources/media/figure_2_3_4.png" width="240"/>

7. You can chose one of the scanned APs or provide your own AP crednetials. Provided credentials are sent to the WFI32-IoT board as you press **SEND**. 

<img src="resources/media/figure_2_3_5.png" width="240"/>
<img src="resources/media/figure_2_3_6.png" width="240"/>

8. Once you go back in the app, WFI32-IoT board will apply new credentilas.
9. The **blue LED** will light up once a successful connection to the Wi-Fi Access Point is made.


### 2.4 Visualizing Cloud Data in Real Time <a name="chapter2.4"></a>
Out of the box, all WFI32-IoT boards are pre-registered to Microchip’s AWS Cloud sandbox account. This account is set up for demonstration purposes only. All data gathered by the sensors of the WFI32-IoT boards are published to the Microchip sandbox account and can be identified by the following details:
Project ID	|	Region
---		|	---
pic-IoT		|	us-central1

There is no permanent storage, or collection of the data published by the boards connected to the Microchip sandbox account. The full storage catalog of the AWS Cloud features, such as data storage/ retention, can be available to the user with use of the board once removed from the demo environment and the associated Device ID/Public Key has been migrated to a private account.

#### Publishing Data to the AWS Cloud
A MQTT PUBLISH packet is always sent to the MQTT broker using a specific topic. The WFI32-IoT board publishes messages using the topic ‘{deviceID}/sensors’ in communication to the AWS Cloud. The messages published on this topic contain the real-time data obtained from the on-board light and temperature sensors. It does not perform any averaging of data, which is done to allow instantly visible changes on the webpage. The frequency of sending a PUBLISH packet can be decided by the user application. The application is written such that the sensor data is published to the Cloud every one second. 

#### Viewing the published messages
Once the board is connected to a Wi-Fi access point, and has established a socket connection to the Cloud, the WFI32-IoT webpage will show a real-time graph of the data captured from the on-board light and temperature sensors. Data is sent as a MQTT PUBLISH packet from the board to the cloud through a JSON object.
The ASCII string is formatted as follows:
{‘Light’: XXX, ‘Temp’: YYY },
where XXX and YYY are numerical values expressed in decimal notation. The yellow LED on the board is turned on for 500 ms, every one second to indicate that the board is publishing data.
 <p align="center">
<img src="resources/media/figure_2_4_1.png" width="480"/>
</p>

#### Subscribing to topics
In addition to publishing its own data, the WFI32-IoT boards are also capable of subscribing to a topic, after which it will receive data from the AWS Cloud whenever data with that topic is published to the broker server. Subscribing to topics is desired when the receiver is interested in the information sent to the broker by other connected client devices publishing data using the subscribed topic. After sending a SUBSCRIBE packet, all the messages published on the specific topic of subscription are received by the board. As of now, the board subscribes to the '$aws/things/{deviceID}/shadow/update/#' topic but will only make use of data comes via the '$aws/things/{deviceID}/shadow/update/' topic.

#### Sending the messages
The pic-iot.com webpage URL displays a section “Control your device” below the Light and Temperature graphs. The Toggle button is used to send the switch value to the WFI32-IoT board.
 <p align="center">
<img src="resources/media/figure_2_4_2.png" width="480"/>
</p>

#### Viewing Messages Received on Subscribed Topic
The toggle switch value corresponds to a short forced ON/OFF state to the yellow LED on the WFI32-IoT board. The LED will stay ON/OFF for a short time depending on the position of the toggle switch. After that, the LED will resume normal behavior, blinking to indicate the transmission sensor data through PUBLISH packets. In addition to the yellow LED behavior, the values of the toggle and text field values can be viewed on a Serial Terminal application (such as Tera Term, Realterm, PuTTy, etc.).
Figure 2-10. Viewing Messages on a Serial Terminal

There is no permanent storage, or collection of the data published by the boards connected through the Microchip sandbox account. The full storage features available by the AWS Cloud are available to the user after the board has been removed from the demo environment (Microchip Sandbox) and migrated to a private account.

### 2.5 Voice Control <a name="chapter2.5"></a>
Out of the box, WFI32-IoT board allows you to control an on board LED using Amazon Alexa voice control. Please follow the below steps to do that. Please note that supported browsers inlcudes Google Chrome, Mozilla Firefox, Safari, and Microsoft Edge while Internet Explorer is not supported. Please also note that only devices registered to the Microchip Cloud account can be registered for voice control and controlled via the voice skills. In case of registration errors, please contact [Microchip support](http://microchip.com/support)

#### Create an account
Create an account and log-in to the [device registration page](https://microchiptech.github.io/mchpiotvoice/). You can also reach out to this page using the file 'VOICE.HTM' on the MSD.

#### Enter thing name
* Enter your thing name and a friendly name and claim your device by registering it. Thing name can be found at the top of the demo web page just above the temperature graph
* Successfully claimed devices will show up in the device listing in the left side panel.

#### Use Amazon Alexa® app
Using Amazon Alexa® app, enable the skill '<img src="resources/media/figure_2_5_1.png" width="30" /> [Microchip IoT](https://www.amazon.com/gp/product/B08B5THZH5?ref&ref=cm_sw_em_r_as_dp_iuhtblwcTlYL4)' and add the dev board as a smart home device.   

#### Control the device
You can now control the on-board Yellow LED with voice commands using the friendly name provided while claiming the device earlier:
> Alexa, turn on the light
   
 You can find out more information about connecting a smart home device to Alexa from [this link](http://tinyurl.com/alexa-smart-home)

### 2.6 Configuring Other Settings <a name="chapter2.6"></a>
While the WFI32-IoT development board comes out of the box fully programmed and provisioned, the user can still control aspects of the application firmware behavior through the USB interface. There are three methods to do this: WIFI.CFG (reconfigure credentials) drag and drop using the mass storage feature, commands through the serial command line interface (CLI), or using MPLAB X® IDE, and the on-board programmer/debugger PKOB4.

#### Serial USB Interface
This interface may be used to provide application diagnostic information. To access this interface, use any preferred serial terminal application (i.e. Teraterm, Coolterm, PuTTy) and open the serial port labeled Curiosity Virtual COM port, with the following settings:


Table 2-2. Serial USB Interface Settings

Baud Rate	115200
Data	8-Bit
Parity Bit	None
Stop Bit	1 Bit
Flow Control	None
Local Echo	ON
Transmit Protocol	CR+LF (Carriage Return + Line Feed)
Note:  For users of the Windows environment, the USB serial interface requires the installation of a USB serial port driver, included in the installation of the MPLAB X IDE.

#### On-board Programmer/Debugger Interface
For users familiar with the MPLAB X IDE, the WFI32-IoT boards can also be programmed, and/or debugged directly via these IDEs standard operations. The WFI32-IoT boards are automatically detected by the MPLAB X enabling full programming and debugging through the on-board PKOB4 interface. For code generation, see Chapter 3 on how to generate a sample application code in MCC.
 

 

## Chapter 5: Code Generation <a name="chapter5"></a>

### 5.1 Generating code from MHC  <a name="chapter5.1"></a>
The source code of the PIC-IoT WG development boards is available using the MPLAB Code Configurator (MCC) in MPLAB X IDE.

3.1.1	Generating the demo
Once the board is connected to the host machine and MPLAB X is launched, follow these steps to generate microcontroller code for it.

3.1.1.1	Generating the demo
1.	Create a new Standalone project (see the Figure 3-1) in MPLAB X 5.15 using the PIC24FJ128GA705 as device (see the Figure 3-2); the PKOB nano as programming tool (see Figure 3-3); and the XC16 as compiler (see Figure 3-4). Finally, name the MPLAB project and its location (see Figure 3-5). The Start page will then appear.
2.	On the MPLAB X toolbar, look for and click the MCC Icon ( ) or click Tools>Embedded>MPLAB X Code Configurator v3 Open/Close.
3.	Under Device Resources, scroll down to the ‘Internet of Things’ header. Under Examples, double- click on PIC-IoT WG Sensor Node (see Figure 3-6).
Figure 3-1. New Project
 

 
Figure 3-2. Selecting a Device
Figure 3-3. Selecting a Programmer
 
 

 
Figure 3-4. Selecting a Compiler
Figure 3-5. Naming a New Project
 
 

 
Figure 3-6. MCC Start Page
3.1.1.2	Configuring the Settings of the Project
The example module makes use of multiple libraries and peripherals. To configure the libraries, double- click on each library in the Device Resources window (see the Figure 3-7) to view their setup windows.
Figure 3-7. MCHP-IoT Peripheral Libraries
 

 
3.1.1.3	Component Library and Peripherals
•	CryptoAuthLib
–	The Crypto Authentication Library (CryptoAuthLib) is not available for user modification but it shows the macros that need to be enabled for the Crypto Authentication functionalities for the PIC-IoT board to work. It also indicates the communication settings between the ECC608 chip and the embedded microcontroller on board (see the Figure 3-8).
•	WINC
–	Under the WINC library, the user can configure the default SSID, password, and the authentication type used for the network to which the board will be connected (see the Figure 3-9).
•	Cloud Services - Google
–	The Cloud Services Google library contains settings for developers to use customer Google Cloud Project credentials by modifying the Project ID, Project Region, and Registry ID. These credentials configure which Google Cloud Server the Socket connection is established over. The default credentials used are those from the public Microchip sandbox project (see the Figure
3-10) used for the PIC-IoT demonstration application.
•	Message Queuing Telemetry Transport (MQTT)
–	MQTT is used as a messaging protocol that operates on top of a TCP/UDP connection to transporting data between client and broker over the Cloud. In MCC, the user can change their MQTT host and connection time-out duration (see the Figure 3-11).
Figure 3-8. CryptoAuthLib MCC
 

 
Figure 3-9. WINC MCC
 

 
Figure 3-10. Cloud Services Google MCC
 

 
Figure 3-11. MQTT MCC
3.1.1.4	Generating MCC Files and Programming the Board
•	Once the changes are made, click the Generate button on the left-hand corner of the window (see the Figure 3-12) and wait for the generation to complete.
•	For the code to work at optimal level, the user needs to change the optimization settings for the compilers. Right-click on the project name and select Properties. Click XC16 in the left-hand sidebar (see the Figure 3-13).
•	Select Most Useful Options in the Categories drop-down menu (see the Figure 3-14). Select Optimization Level "s" from the drop-down menu. Click the Apply button and then OK.
•	Click the Make and Program Device button near the middle of the toolbar. Make sure the board is connected to the system during programming.
 

 
Figure 3-12. Generating MCC Code

Figure 3-13. Project Properties
 
 

 
Figure 3-14. Optimization Settings
 

## Chapter 6: FAQs, Tips and Troubleshooting <a name="chapter6"></a>

### 6.1 FAQs and Tips  <a name="chapter6.1"></a>
1.	How can I change the Wi-Fi configuration?
There are four ways to do it:
-	Connect to the USB and click the "click-me" file to reach the https://www.pic-iot.com/ page (with the /device/{deviceID} suffix). Then enter the new credentials in the web form. Download the resulting file to the CURIOSITY drive. Read more in the 2.3.1 Via PIC-IoT Webpage section.
-	Connect to the USB and open a serial port terminal (Windows users will need to install serial port drivers). From the command line, use the Wi-fi command. Read more in the 2.3.1 Via PIC-IoT Webpage section.
-	Press the SW0 button while powering-up the board. The WINC will turn to Access Point mode. Connect the laptop or phone to it and fill in the online form. See the 2.3 Connecting the Board to Wi-Fi Networks section for details.
-	Use MCC to re-build the project after changing the default Wi-Fi configuration in the WINC module. Re-program the board using MPLAB X or drag and drop the new image to the CURIOSITY drive. Further details can be found in the 3.1 Generating code from MCC section.
2.	How can I change the Wi-Fi credentials using the online form without exposing the details to security threats?
Although it appears in your browser, the Wi-Fi credential setup form does not transfer any information to third parties. A small text file (WIFI.CFG) is created (this can also be done manually using any text editor) and it is recommended that you save it directly to the CURIOSITY drive. (Since the browser settings vary according to platform and personal preferences, you might have to change them or perform a drag and drop from your default download folder). Even though it looks like the WIFI.CFG file is now stored on the CURIOSITY drive, this is just an artifact of your operating system (caching). No file is permanently recorded and the information contained is immediately used to update the Wi-Fi module settings. These settings will be maintained after subsequent power cycles of the PIC-IoT board, but the file will disappear.
3.	Can I use my phone/tablet alone to perform the demo?
Assuming you have a way to provide power to the board (a USB back-up battery, a USB charger, a Li-Ion battery, or other 3.3V-5V power supply), you can scan the QR code (on a sticker under the board, next to the Microchip and Google color logos) using any smartphone camera (old operating system versions might still require a separate app) and open the resulting link in the smartphone browser.
4.	I scanned the bar code with my phone/tablet but nothing happened?
Make sure you are scanning the QR code present on the sticker under the WFI32-IoT WG board. You can recognize it by the distinguishing squares on the three of its corners and its proximity (same sticker) to the MCHP and Google logos (in color). Although there are also other bar codes present on the Wi-Fi module and/or the anti-static bag the board came with, those are not QR codes
5.	Which battery is recommended to be used with the IoT WG Sensor Node board?
We recommend Li-Ion or Li-Poly batteries with 500 mAh capacity and 3.7 V nominal. Coin cell batteries are not applicable as they cannot provide enough current for the Wi-Fi module when in transmission (short bursts due to excess of 200 mA).
Pro Tips:
 

 
The following steps are not required for operating the IoT WG Sensor Node board but will increase significantly the possibility of positive results.
1.	Get a USB cable with all the four wires connected. There are a lot of non-compliant USB cables out there that are providing only 5V power (two wires). How can you verify it? Plug the board into your laptop and check in the File Manager (Finder) for the presence of a new hard drive (named CURIOSITY). If you do not see it, popping up after a second or two, the cable is not the appropriate one.
2.	Prepare your Wi-Fi router for the demo. The easiest way to go is to setup your phone as a hotspot. You will want to assign to it the following credentials, name (SSID): MCHP.IOT and password: microchip (WPA 2 is assumed, do no use WEP nor OPEN.) This Wi-Fi configuration is the factory default for all boards, so it will minimize the effort for first users. If preparing for a (medium/large) classroom demo, you will want to setup a proper Wi-Fi router (2.4 GHz) instead. This will give you range and capacity while using the same Wi-Fi credentials, if possible.
3.	Make Google Chrome or Firefox your default web browser (if only for the day of the demo/visit). Safari works well on MACs. Even if the latest versions of Internet Explorer are known to work OK on Windows 10, the chances decrease significantly as the Windows user base is less uniform and you might find some (customer) corporate policies to maintain antique versions for legacy application compatibility reasons.
4.	Ensure no pop-up blockers or other anti-virus extensions to the browsers are active. These can and will interfere with the script that is at the heart of the https://www.pic-iot.com/ microsite. Often, these can be selectively disabled for that specific webpage.
5.	Count the number of cellphones in the room. If it is more than 50, do not try a live demo. The board will eventually establish a connection but it might not happen as promptly and cleanly as you would like to demonstrate. The Wi-Fi pollution is a serious problem at large events, fares and in general in any large public space nowadays.
6.	Check your setup ahead of time and verify you have 4G (or LTE if using the phone hotspot), a full charge and no firewall (if using a router) blocking access to the Google Cloud server (try opening any secure webpage such as: https://microchip.com. If you get the homepage, the way is clear).


### 6.2 LED Status Troubleshooting  <a name="chapter6.2"></a>
Table 5-1. Troubleshooting and Diagnostics

LED Sequence	Description	Diagnosis	Action

 	No LEDs are On	Board is not programmed	Download image .hex file from the bottom of the microsite page.

 	Only Red LED is ON	Board is not connected to the Wi-Fi Access Point	•	Verify Wi-Fi credentials
•	Confirm Access Point is available
 

 

...........continued
LED Sequence	Description	Diagnosis	Action

 	Blue LED BLINKS with all other LEDs OFF	Board is in Soft AP mode	•	Must connect to board using phone, or a network capable device
•	Send updated credentials via Soft AP

 	Green LED is BLINKING; Other LEDs
are OFF	Board is using DEFAULT CREDENTIALS	•	Allow board to connect to Access Point
•	Update CREDENTIALS through CLI if DEFAULTS selection was invalid

 	Blue and Red LEDs are ON	Board is not connected to the Google IoT Cloud Servers	•	Verify MQTT required ports
•	Verify project credentials
•	Check local network firewall settings
•	Use tethered cell phone or laptop connection for internet

 	Blue, Green and Red LEDS are ON	Sensor data is not being published to Cloud	•	Verify device registration to the project
•	Check Google account settings

 	Blue and Green LEDs are ON. Yellow LED is BLINKING	Everything is working	•	Nothing to be done

OR	Blue and Green LEDs are ON. Yellow LED held HIGH/LOW	Subscribe topic toggle value received	•	Nothing to do.
•	LED will reflect ‘Toggle’ value LED behavior returns to normal after HOLD PERIOD
 

 
Table 5-2. PKOB nano LED Troubleshooting

LED Sequence	Description	Diagnosis	Action

 	PKOB nano LED is OFF	Board is not powered	•	Check the USB connection
•	Replace the board

 	PKOB nano LED is ON but CURIOSITY driver not found	Faulty USB connection	•	Check the PC device manager
•	Replace the USB cable
•	Replace the USB connector
 

## Chapter 7: Appendix <a name="chapter7"></a>

### 7.1 Relevant Links  <a name="chapter7.1"></a>
The following list contains links to the most relevant documents and software for the PIC-IoT WG board. For those accessing the electronic version of this document, the underlined labels are clickable and will redirect to the appropriate website..
•	MPLAB® X IDE - Free IDE to develop applications for Microchip microcontrollers and digital signal controllers.
•	MPLAB® Code Configurator (MCC) - a free, graphical programming environment that generates seamless, easy-to-understand C code to be inserted into the project. Using an intuitive interface, it enables and configures a rich set of peripherals and functions specific to the application.
•	Microchip Sample Store - Microchip sample store where you can order samples of devices.
•	Data Visualizer - Data Visualizer is a program used for processing and visualizing data. The Data Visualizer can receive data from various sources such as the Embedded Debugger Data Gateway Interface found on Xplained Pro boards and COM ports.
•		Microchip Wi-Fi Provisioning app - Wi-Fi Provisioning app is a tool to provision Microchip Wi-Fi products to connect to a Wi-Fi Network. This can be also used to send raw data to the device which user application can use for various aspects.
