# ESP32 wifi module for Punch HW project
=====================================

## ESP32-Serial-Bridge
Transparent WiFi (TCP) to UART Bridge, supports STATION WiFi modes. The .ino file is the code for the ESP32. Use PlatformIO and VSCode IDE for ESP32 to compile and upload it to the ESP32.

### Tools to build and upload
* Visual studio and PlatformIO extenion.
	* In VC/PlatformIO Home, add library espressif8266 or espressif32
	* Open the Wifi project/folder that you checked out from gif.
* Hardware Connectivity <todo>
	* punch usb <--> PC
	* punch wifi esp32 uart <--> cp2102 usb-uart convertor <--> PC (figure1)
	* connect the boot pins together then upload vs. VSCode. Remember to reset the HW.
	* click Upload button in the PlatformIO toolbar
	* disconnect eh boot pins. reset.

figure 1:								___ usb
							 ` Tx		___
					mainmcu	 ` Rx	>reverse on other end(cp2102)
							 ` Gnd
		`  esp boot					_______
		`							| mcu |
					' macimcu boot	|_f103|
espmcu	` Rx		'						
		` Tx	>reverse on other end(cp2102)
		` Gnd


## Api interface and note
* When reset, it  trying to connect to the host server listening on the following connection/port. It will timeout in 20 second and go to USB only mode. 

use the SampleApp to modify them: 
Accesspoint (modifiable thru Punch's api sample app):
AP SSID: eMediat NET_EXT
AP Password: thanhtruong
IPAdress: 192.168.1.10:9500 <-> STM32F103RB mcu COM2

* Modify connection infor:
	* Power on Punch HW (wait for 20 second for it go into USB mode)
	* Launch SampleApp
	* execute OpenDevice on the COMx port that Punch HW connective to.
	* execute all/some of the Wifi api (bottom of the list)
	* reset HW
	* API: CloseDevice then OpenDevice in TCP mode with the connction's infor. set above.
	* Now you are communication with Punch HW in Wifi-mode (and USB is active if you want to monitor with other serial app)


===============================================================

