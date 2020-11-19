# Application Deep Dive: Secure Cloud Connectivity and Voice Control Demo for Microchip WFI32-IoT Board .

Devices: **| PIC32 WFI32E | WFI32 | Trust\&Go (ECC608) |**

Features: **| Secure Cloud connectivity | Voice Control |**

[![Latest release](https://img.shields.io/github/v/release/MicrochipTech/PIC32MZW1_Curiosity_OOB?include_prereleases&sort=semver&style=for-the-badge)](https://github.com/MicrochipTech/PIC32MZW1_Curiosity_OOB/releases/latest)
[![Latest release date](https://img.shields.io/github/release-date/MicrochipTech/PIC32MZW1_Curiosity_OOB?style=for-the-badge)](https://github.com/MicrochipTech/PIC32MZW1_Curiosity_OOB/releases/latest)

The WFI32-IoT board comes pre-programmed and configured for demonstrating the connectivity to the AWS Cloud IoT Core. The demo uses **AWS C SDK version 4.0** to establish MQTT connection to AWS broker, subscribe to cloud topic/s and publish to the cloud.

<p align="center">
<img src="resources/media/board1.png"/>
</p>
 
## Table of Contents

1. [Requirements](#chapter1)
2. [Application Scope](#chapter2)
3. [Application Structure](#chapter3)
4. [Application Description](#chapter4)
5. [Secure Provisioning & Transport Layer Security](#chapter5)
6. [Understanding the Device Shadow in AWS ](#chapter6)
7. [Detailed operation](#chapter7)
 
## 1. Requirements <a name="Chapter1"></a>

* **MPLAB® X Integrated Development Environment (IDE) v5.40 or later**
MPLAB® X IDE is a computer software program based on the open source NetBeans IDE from Oracle. It is used to develop applications for Microchip microcontrollers and digital signal controllers. It runs on Windows®, Mac OS® and Linux®. 
For the latest version, please refer to: [MPLAB-X](https://www.microchip.com/mplab/mplab-x-ide)

* **MPLAB® XC32 Compiler v2.41 or later**
MPLAB® XC compilers support all of Microchip’s PIC, AVR and dsPIC devices where the code is written in the C programming language. XC32 is the recommended compiler for 32-bit PIC MCUs. In this lab, as well as with the succeeding labs, you will be using MPLAB® XC32 for an PIC MCU.
For the latest version, please refer to: [XC-Compiler](https://www.microchip.com/mplab/compilers)

* **WFI32-IoT Board**
The WFI32-IoT board is a compact, easy-to-use development board that supports rapid prototyping of IoT devices and demonstrates cloud connectivity with voice control enablement. This kit is also a useful tool to evaluate the features of WFI32E01PC, the single-chip Wi-Fi module. The board also includes an on-board debugger and requires no external hardware to program and debug the MCU.

---

## 2. Application Scope <a name="Chapter2"></a>
The WFI32-IoT-IoT development board has been created with the intention of demostrating a one source solution for evaluation of existing cloud provider solutions. 
This example end-device leverages the catalog of devices, and libraries provided through Microchip's extensive product line to showcase a basic Internet of Things product connection. Data exchange between server and in field device is implemented using on board sensors for temperature and light value observations. Behavior actions are demonstrated through visual indication of the 'Data' LED as controlled through the Web based APIs. 

General Out-Of-Box operation is as described below:
1. Use the WFI32E01PC single-chip WiFi module to establish local WiFi connection to Router/Switch or Network source. The **Blue 'Wi-Fi' LED** is used to indicate this status. 
2. The on-module ATECC608A HSM is used to establishe a Secure (TLS) Socket Connection with select Cloud Provider using a TCP connection. The **Green 'Connect' LED** is used to indicate this status
3. Using **AWS C SDK**, data is exchanged between client (end-device) and broker (cloud). 
4. Sensor Data is sent as Telemetry Data between device and broker at a periodic rate of 1 Second. The **Yellow 'Data' LED** blinks to indicate this status. 
5. Capture of Data sent from Broker to Device can be observed through a Serial terminal when USB-Micro is connected to WFI32-IoT board. 
6. Behavior variation can be observed on the 'Data' LED when triggered through the web based API and sent through the broker to end device.

**Note**: The **SW0 & SW1** user buttons have no effect outside of variation of start-up operation where:
* **SW0** held: Enter Soft AP mode.
* **SW0 & SW1** held: Use default Wi-Fi credentials {**MCHP.IOT, microchip**}

**Note**: The **Red 'Data' LED** remaining on may indicate a hardware fault or a non connected Wi-Fi.

---

## 3. Application Structure <a name="Chapter3"></a>

The application runs two application OS tasks/threads with multiple underlying logical modules for better code organizing.

| OS Task Name         	| App Logical Module		| Source/Header Files 	| Role                                                                          	|
| --------------------- | ----------------------------- | --------------------- | ------------------------------------------------------------------------------------- |
| **_APP_Tasks**    	| **APP**			| app.c/.h		| The main/central module that manages **Wi-Fi functionality**.              		|
| 		     	| **APP_WIFI_PROV**		| app_wifi_prov.c/.h	| Manages **AP provisioning functionality** to provision the device using AP mode	|
|		  	| **APP_USB_MSD**<sup>(*)</sup>	| app_usb_msd.c/.h	| Manages **Mass Storage Device functionality**  					|
|		  	| **APP_CTRL**			| app_ctrl.c.h		| Manages device **Control operations** including LED management and sensors access	|
| **_APP_AWS_Tasks**    | **APP_AWS**			| app_aws.c/.h		| Manages **AWS cloud connection/subscribe/publish functionality**			|

(*) **Mass Storage Device** gives access to:
* Configure the device for Wi-Fi connection via **WIFI.CFG**.
* configure the device for cloud connection via **CLOUD.JSON**.
* Demo Webpage via **CLICK-ME.HTM**.
* Device registration for Alexa Voice control via **VOICE.HTM**.

---

## 4. Application Description <a name="Chapter4"></a>

### AWS Cloud
* Publish payload for sensor data (telemetry)
	* topic: ``<thingName>/sensors ``
	* payload: 
	```json
	{
	  "Light": lightValue,
	  "Temp": temperatureValue
	} 
	```
* Device publishes payload to update the Device Shadow
	* topic: ``$aws/things/<thingName>/shadow/update``
	* payload:
	```json
	{
	  "state":
	  {
	       "reported":
	       {
		    "toggle": updatedToggleValue
	       }
	  }
	}
     ```
* Web Interface publishes payload to Device Shadow
	* topic: ``$aws/things/<thingName>/shadow/update``
	* payload:
	```json
	{
	  "state":
	  {
	       "desired":
	       {
		     "toggle": toBeUpdatedToggleValue
	       }
	  }
	}
	```
* Device subscribes to delta to receive actionable changes
	* topic: ``$aws/things/<thingName>/shadow/update/delta``
	* payload:  
	```json
	{
	   "state":
	  {
	       "Light": lightValue,
	  }
	}
	```
* The PIC IoT development board publishes data from the on-board light and temperature sensor every 1 second to the cloud.
* The data received over the subscribed topic is displayed on a serial terminal and **YELLOW LED blinks** accordingly.

### Establish MQTT connection to cloud  
* The C code for establishing MQTT connection is available in **src/app_aws.c** file.
* AWS C SDK API ``IotMqtt_Connect`` is called for publishing data to the cloud.

### Sending MQTT publish packets  
* The C code for sending MQTT publish packets is available in **src/app_aws.c** file.
* The API ``static int publishMessages()`` is responsible for publishing data at an interval of 1 second.
* AWS C SDK API ``IotMqtt_PublishAsync`` is called for publishing data to the cloud.

### Subscribe to topic/s
* The C code for subscribing to topic/s is available in **src/app_aws.c** file.
* AWS C SDK API ``IotMqtt_SubscribeSync`` is called for subscribing to cloud topic/s.

### Processing Packets received over subscribed topic
* The C code for processing MQTT publish packets received over the subscribed topic is available in **src/app_aws.c** file.
* The ``static void mqttSubscriptionCallback( void * param1, IotMqttCallbackParam_t * const pPublish )`` function is used for processing packets published over the subscribed topic.

---

## 5. Secure Provisioning & Transport Layer Security <a name="Chapter5"></a>

1. The WFI32-IoT board is shipped pre-provisioned for coordination with the AWS Cloud system.
2. Security is achieved by using the WolfSSL Transport Layer Security (TLS) stack configured within Harmony 3 eco-system.
3. A Pre-Manufacturing process has configured the appropriate slot locations on the ATECC608A HSM.
4. All required certificates used for signing and authentication have been written to and 'locked' into allocated slots.
	* This process is achieved through: [Trust&Go](https://www.microchip.com/design-centers/security-ics/trust-platform/trust-go)
	* Full scope of support for Secure Aspects of development can be found here: [Trust Platform](https://www.microchip.com/design-centers/security-ics/trust-platform)
5. This process has been performed to allow for an Out Of Box (OOB) operation of the WFI32-IoT board along with supporting web page.
6. Shadow Topic are a key application feature being supported through WFI32-IoT demo. Further reference can be found here: [Shadow MQTT Topics](https://docs.aws.amazon.com/iot/latest/developerguide/device-shadow-mqtt.html)

---

## 6. Understanding the Device Shadow in AWS <a name="Chapter6"></a>

1. The AWS broker allows for the use of Shadow Topics. The Shadow Topics are used to retain a specific value within the Broker so that End-Device status updates can be managed.
	* Shadow Topics are used to restore the state of variables or applications.
	* Shadow Topics retain expected values and report if Published data reflects a difference in value.
	* When difference exists, the status of the delta is reported to those subscribed to appropriate topic messages.

<img src="resources/media/image17.png"/>

2. Updates to the device shadow are published on ``$aws/things/<ThingName>/shadow/update`` topic. When a message is sent to the board by changing the value of the **toggle** fields in **Control Your Device** section:
	* This message is published on the ``$aws/things/<ThingName>/shadow/update`` topic.
	* If the current value of toggle in the device shadow is different from the toggle value present in the AWS Device Shadow, the AWS Shadow service reports this change to the device by publishing a message on ``$aws/things/<ThingName>/shadow/update/delta`` topic.
3. AWS IoT Core publishes a delta topic message if there is a difference between the reported and desired states. The device would have already subscribed to the delta topic.
4. You can read more about AWS device shadows (here.)[https://docs.aws.amazon.com/iot/latest/developerguide/device-shadow-data-flow.html]

---

## 7. Connecting to Your Cloud Instance

By default, the demo connects to an instance of AWS IoT maintained by Microchip. The demo lets you move the device connection between your cloud instance, and the Microchip maintained AWS IoT instance without a firmware change. Perform the following steps to get the device connected to your own cloud instance.

1.  Create an AWS account or log in to your existing AWS account.
    - Please refer to [Set up your AWS account](https://docs.aws.amazon.com/iot/latest/developerguide/setting-up.html) and [Create AWS IoT resources](https://docs.aws.amazon.com/iot/latest/developerguide/create-iot-resources.html) for details.

2.  Navigate to [IoT Core console](https://console.aws.amazon.com/iot/) \> Manage \> Things and click on “**_Create_**” / “**_Register a Thing_**”

<p align="center">
<img src="resources/media/image7.png" width=480/>
</p>

3.  Select “**_Create a single thing_**”

4.  For thing name, copy and paste the thing name from the original demo web-app. This thing name originates from the device certificate and is used by the firmware to send messages to a unique topic.

<p align="center">
<img src="resources/media/image8.png" width=480 />
</p>

5.  Select defaults for the other fields and click “Next” at the bottom of the page.

6.  Select “**_Create thing without certificate_**” in the next page.

7.  Go to “**_Secure_**” \> “**_Policies_**” and select “**_Create a Policy_**”

<p align="center">
<img src="resources/media/image9.png" width=480 />
</p>

8.  Create a new policy which allows all connected devices to perform all actions without restrictions

  > :x: &nbsp; **_Note_**: This policy grants unrestricted access for all iot operations, and is to be used only in a development environment. For non-dev environments, all devices in your fleet must have credentials with privileges that authorize intended actions only, which include (but not limited to) AWS IoT MQTT actions such as publishing messages or subscribing to topics with specific scope and context. The specific permission policies can vary for your use cases. Identify the permission policies that best meet your business and security requirements.Please refer to [sample policies](https://docs.aws.amazon.com/iot/latest/developerguide/example-iot-policies.html) and [security best practices](https://docs.aws.amazon.com/iot/latest/developerguide/security-best-practices.html)

| Item               | Policy Parameter |
| ------------------ | ---------------- |
| **_Name_**         | allowAll         |
| **_Action_**       | iot:\*           |
| **_Resource Arn_** | \*               |
| **_Effect_**       | Allow            |

<p align="center">
<img src="resources/media/image10.png" width=480/>
</p>

9.  Navigate to **_Certificates_** \> **_Create a certificate_**

<p align="center">
<img src="resources/media/image11.png" width=480/>
</p>

10. Select Create with “**_Get Started_**” under “**_Use my certificate_**”.

11. In the next screen, click “**_Next_**” without making any selections.

12. Click on “**_Select certificates_**”

13. In the MSD enumerated when the Curiosity Board is plugged in, you can find a “**_.cer_**” file with an alphanumeric name. Select this file when prompted to select a certificate.

14. Select “**_Activate all_**” and click “**_Register certificates_**”

<p align="center">
<img src="resources/media/image12.png" width=480/>
</p>

15. Select the certificate and

    1.  Click **_Attach policy_** and select the “allowAll” policy we created

    2.  Click **_Attach thing_** and choose the *thing* we created

<p align="center">
<img src="resources/media/image13.png" width=240/>
</p>

16. Navigate to “**_Settings_**” and copy the endpoint URL

<p align="center">
<img src="resources/media/image14.png" width=480/>
</p>

17. Navigate to the MSD and open “**_cloud.json_**”

  > :information_source: &nbsp; While editing `cloud.json` or `WIFI.CFG` manually, it is recommended to use ***notepad.exe*** . Other editors like Notepad++ can damage the underlying FAT12 FS. You can read more about this generic issue in the discussion [here](https://github.com/adafruit/circuitpython/issues/111). In case you come across this, please re-flash the image to recover.

18. Replace the “**_brokerName_**” attribute with the endpoint URL.

19. Reset the device. Now, the device will connect to your own cloud instance.

20. In the AWS IoT console, navigate to “**_test_**” and subscribe to topic “**_+/sensors_**”

<p align="center">
<img src="resources/media/image15.png" width=480 />
</p>

21. You will be able to observe periodic temperature data coming into the console from your device.

22. To control the Green LED, publish the following message:

<table>
<thead>
<tr class="header">
<th><strong>Topic</strong></th>
<td>$aws/things/<em><strong>thingName</strong></em>/shadow/update</td>
</tr>
</thead>
<tbody>
<tr class="odd">
<td><strong>Payload</strong></td>
<td><pre>
{
  "state": {
    "desired": {
      "toggle":1
    }
  }
}
</pre></td>
</tr>
</tbody>
</table>

Depending on the value of “**_toggle_**” (1/0) , the Green LED will be ON/OFF.

---

### Detailed Operation
1. There are three possible variations within application behavior possible by holding push buttons on startup
	* Default behavior: No Button Pressed
	* Soft AP: **SW0** is Held on startup (see description farther in document)
	* Restore default Wi-Fi credentials: **SW0 & SW1** Held on startup. This state is reflected by **BLINKING GREEN LED** until a Wi-Fi connection is established. 

  1. Connect board to PC using USB-micro cable. 

     + The LEDs will **Cycle** upon startup: **BLUE-->GREEN-->YELLOW-->RED**, short delay, **BLUE-->GREEN-->YELLOW-->RED**.

  2. The **BLUE LED** will begin to blink, this indicates the board is attempting to join the local **ACCESS POINT**.

     ![Connecting_2](images/picawsLeverage.png)

  3. Update the Wi-Fi Credentials; upon connecting the blinking will stop, and the **LED** will become **STATIC**. Below are the easiest methods to update credentials.
     + The board will appear on the PC enumerated as a mass storage device under the name **CURIOSITY**. Credentials can be downloaded as the file **WIFI.CFG** using the **CLICK-ME.HTM** file stored on the **CURIOSITY** device.
     
     ![URL Hosted Credentials](images/wifiCredentialsWeb.png)

     + This will launch the URL: https://pic-iot.com/aws/{ThingName}. 
     + After entering credentials, the **.CFG** file is produced through the web browser. No information is shared through the internet. 
     + Drag and Drop, or Copy and Paste the **WIFI.CFG** file onto the **CURIOSITY** device to load new credentials onto the IoT demonstration board.
	 
     
     ![WiFi Config](images/wifiNotepad.png)     
     
     + Use a **Serial Terminal** to update the WiFi Credentials loaded onto the WINC module. Use the Command Line Interface (CLI) supported command ``wifi host_name,pass_code,auth_type`` | host_name/pass_code are entered strings, auth_type is int value: (0: open, 1: WEP, 2: WPA).

     ![Serial Credentials](images/serialWiFi.png)

  4. After becoming connected to the ACCESS POINT, the GREEN LED will begin to blink, this indicates the board is attempting to establish a TCP/IP and MQTT connection with the cloud providing service. The GREEN LED will stop blinking and become STATIC when the TCP and MQTT connection is established.
     + Using the in module TCP/IP stack pre-configured with provisioned credentials; the device establishes a **MQTT** connection with the IoT Broker provider (AWS).
     ![Status Display](images/awsConnection.png)

  5. After successfully establishing MQTT connection, the **YELLOW LED** will blink, indicating data exchanged between the End-Device (PIC-IoT), and BROKER (AWS). 

     ![Telemetry Data](images/PublishDataToAWS.png)

  6. Connect to the www.avr-iot.com/aws/{thingName}, or www.pic-iot.com/aws/{thingName}, device specific website to view publish/subscribe data. 
     + This page can be found via launching the **CLICK-ME.HTM** file on the **CURIOSITY** device.
     + There will be (2) scrolling graphs visible. (1) shows temperature sensor, (1) shows the light sensor value. 
     + Additional graphs can be produced altered through the published topic message.

     ![Telemetry Data](images/awsSensors.png)

  7. **Control Your Device** using the (3) rows beneath the '**Control Your Device**' section used to publish subscription data to end-devices through the broker. 
     + **Only the use of Toggle is supported natively**
     + **Expanding features would require custom written Firmware implementation**
     + These example rows demonstrate options for: Toggle (boolean), Text Field (String), Sliders (integer)

     ![Subscribe Data](images/Actuators.png)

  8. When connection is established with the Broker, the publish message topic will be printed to a serial terminal through the CDC-USB bridge.
     + 9600 is expected Baud Rate.
     + When a topic subscription is received, the payload is printed in JSON format to the terminal.
     + Topic subscription message are sent when the 'Send to device' push button on the webpage is pressed. 

     ![Serial Subscribe Message](images/DeviceStartUpConsole.png)

  9. When the 'Desired' state is updated in the 'Delta' Shadow Topic. 
     + Device which required updates to the 'Desired' state which differs from their last 'Reported' value will receieve a published message on the '.../delta' MQTT topic. 
     + Upon reception the device will report the updated 'Desired' value for the attribute with timestamp on the console.

     ![Delta Subscribe Message 2](images/DeltaToggle.png)

