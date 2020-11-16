# Getting Started Guide: Secure Cloud Connectivity and Voice Control Demo for Microchip WFI32-IoT Board .

Devices: **| PIC32 WFI32E | WFI32 | Trust\&Go (ECC608) |**

Features: **| Secure Cloud connectivity | Voice Control |**

[**Frequently Asked Questions**](https://github.com/MicrochipTech/PIC32MZW1_Curiosity_OOB/blob/master/faq.md)

[![Latest release](https://img.shields.io/github/v/release/MicrochipTech/PIC32MZW1_Curiosity_OOB?include_prereleases&sort=semver&style=for-the-badge)](https://github.com/MicrochipTech/PIC32MZW1_Curiosity_OOB/releases/latest)
[![Latest release date](https://img.shields.io/github/release-date/MicrochipTech/PIC32MZW1_Curiosity_OOB?style=for-the-badge)](https://github.com/MicrochipTech/PIC32MZW1_Curiosity_OOB/releases/latest)
 

## Preface
The WFI32-IoT board is a compact, easy-to-use development board that supports rapid prototyping of IoT devices and demonstrates cloud connectivity with voice control enablement. This kit is also a useful tool to evaluate the features of WFI32E01PC, the single-chip Wi-Fi module.

The WFI32-IoT board contains the PIC32MZW1, a highly integrated IoT system core supporting smart Wi-Fi functionalities and a premium MCU. There are various types of on-board sensors for users to build up their applications. Users can also expand its functionality through MikroElectronika mikroBUS™ Click™ adapter boards. The on-board hardware crypto device can be used to make a secure and authenticated cloud connection.
 
## Table of Contents

1. [Chapter1: Overview](#chapter1)
	1. 	[1.1 Board Layout](#chapter1.1)
	2.	[1.2 LED Indicators](#chapter1.2)
	3.	[1.3 Switch Button Use Cases](#chapter1.3)
2. [Chapter 2: Getting Started](#chapter2)
	1.	[2.1 Connecting the Board to the Host PC](#chapter2.1)
	3.	[2.2 Connecting the Board to Wi-Fi Networks](#chapter2.2)
		1. 	[2.2.1 Via WFI32-IoT Web page](#chapter2.2.1)
		2. 	[2.2.2 Via Soft AP](#chapter2.2.2)
	4.	[2.3 Visualizing Cloud Data in Real Time](#chapter2.3)
	5.	[2.4 Voice Control](#chapter2.4)
6.[FAQs, Tips and Troubleshooting](#chapter6)
	1.	[6.1 FAQs and Tips](#chapter6.1)
	2.	[6.2 LED Status Troubleshooting](#chapter6.2)
7.[Appendix](#chapter7)
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

<img src="resources/media/figure_2_1_1.png"/>

### 2.2 Connecting the Board to Wi-Fi Networks <a name="chapter2.2"></a>

#### 2.2.1 Via WFI32-IoT Webpage <a name="chapter2.2.1"></a>
<img src="resources/media/figure_2_3_1.png" width="480"/>

1. The lower left-hand corner of the web page will show a wireless network connection window.
2. Once the required details are entered, click the **Download Configuration** button. 
3. A file named **WIFI.CFG** (text) file is downloaded to the host PC. 
4. Drag and drop the file to the **CURIOSITY drive** to update the Wi-Fi credentials of the board.
5. Reboot the device.
6. The **blue LED** will light up once a successful connection to the Wi-Fi Access Point is made.

**Important**: The Wi-Fi network SSID and password are limited to 19 characters. Avoid using quotation marks, names, or phrases that begin or end in spaces. The WFI32-IoT board supports only 2.4 GHz networks inline, thus using mobile hotspots to connect the board to the Internet is recommended.

**Note**: Any information entered in the SSID and password fields is not transmitted over the web or to the Microchip or AWS servers. Instead, the information is used locally (within the browser) to generate the WIFI.CFG file.
 
#### 2.2.2 Via Soft AP <a name="chapter2.2.2"></a>
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

### 2.3 Visualizing Cloud Data in Real Time <a name="chapter2.3"></a>

#### Viewing the published messages
<img src="resources/media/figure_2_4_0.png" width="720"/>

1. After connecting to an access point, WFI32-IoT board will try to connect to AWS cloud Miicrochip Sandbox account which is indicated by a **blinking Green LED**.
2. Once connection to cloud is successful, **Green LED** will turn solid.
3. Go to demo webpage (can always be reached out using the file **CLICK-ME.HTM**).
4. The webpage will show a real-time graph of the data captured from the on-board light and temperature sensors.
5. The on board **Yellow LED** will blink for 500ms for each message published successfully to te cloud.

<img src="resources/media/figure_2_4_1.png" width="720"/>

#### Sending messages to the board
1. Click on **What's Next** button below the Temprature and Light graphs.
2. Select **Implement a Cloud-Controlled Actuator** to demostrate cloud performed behaviors.

<img src="resources/media/figure_2_4_2.png" width="720"/>

3. Click on **Learn More** button to expand page interface then Scroll to the bottom of **Step 5** where a panel will read **Control Your Device**.

<img src="resources/media/figure_2_4_3.png" width="720"/>

4. By default only a **Toggle** feature is demostrated. Custome implmentations are described further on above the panel.

<img src="resources/media/figure_2_4_4.png" width="720"/>

5. Click on **Send to device** to send **Toggle** button value. 
6. When **Toggle** button is selected, the **YELLOW LED** will remain on for 3 Seconds. When unselected, the **YELLOW LED** will remain off for 3 Seconds. After the 3 seocnds, the **Yellow LED** will go back to its normal functionality; blinking on each successfull message published to the cloud.

**Note**: Because Toggle manipulates the desired stat, the state must be changed to observe the behavior.

### 2.4 Voice Control <a name="chapter2.4"></a>
Out of the box, WFI32-IoT board allows you to control an on board LED using Amazon Alexa voice control. Please follow the below steps to do that. Please note that supported browsers inlcudes Google Chrome, Mozilla Firefox, Safari, and Microsoft Edge while Internet Explorer is not supported. Please also note that only devices registered to the Microchip Cloud account can be registered for voice control and controlled via the voice skills. In case of registration errors, please contact [Microchip support](http://microchip.com/support)

1. Create an account and log-in to the [device registration page](https://microchiptech.github.io/mchpiotvoice/). You can also reach out to this page using the file **VOICE.HTM** on the MSD.
2. Enter your thing name and a friendly name and claim your device by registering it. Thing name can be found at the top of the demo web page just above the temperature graph
3. Successfully claimed devices will show up in the device listing in the left side panel.
4. Using Amazon Alexa® app, enable the skill '<img src="resources/media/figure_2_5_1.png" width="30" /> [Microchip IoT](https://www.amazon.com/gp/product/B08B5THZH5?ref&ref=cm_sw_em_r_as_dp_iuhtblwcTlYL4)' and add the dev board as a smart home device.   
5. You can now control the on-board **Yellow LED** with voice commands using the friendly name provided while claiming the device earlier:
> Alexa, turn on the light
   
 **Note**: You can find out more information about connecting a smart home device to Alexa from [this link](http://tinyurl.com/alexa-smart-home)
 

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
