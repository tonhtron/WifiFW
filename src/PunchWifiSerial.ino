#include "config.h"
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Preferences.h>

#ifdef OTA_HANDLER
#include <ArduinoOTA.h>
#endif // OTA_HANDLER

//-----------------------------------------------------
#define wifi_max_try 10             // Number of try
#define duration_deep_sleep 10      // Sleep until next try

Preferences preferences;//save none-volatile data

//*** HardwareSerial Serial; Default auto create to/from wifi
HardwareSerial serialMcu(1);
WiFiClient client;//me

uint8_t buf[bufferSize];
uint16_t nBuf = 0;//len of buf[]

uint8_t arMsg[64];
int msgsize = 0;

// ********************* Helper functions **********************************
void print_wakeup_reason() {
	esp_sleep_wakeup_cause_t wakeup_reason;

	wakeup_reason = esp_sleep_get_wakeup_cause();

	switch (wakeup_reason)
	{
	case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
	case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
	case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
	case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
	case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
	default: Serial.printf("Wakeup was not caused by deep sleep: %d\r\n", wakeup_reason); break;
	}
}
	
void print_wifi_error() {
	switch (WiFi.status())
	{
	case WL_IDLE_STATUS: Serial.println("WL_IDLE_STATUS"); break;
	case WL_NO_SSID_AVAIL: Serial.println("WL_NO_SSID_AVAIL"); break;
	case WL_CONNECT_FAILED: Serial.println("WL_CONNECT_FAILED"); break;
	case WL_DISCONNECTED: Serial.println("WL_DISCONNECTED"); break;
	default: Serial.printf("No know WiFi error"); break;
	}
}

//
//********************* SETUP ****************************************
//
void setup() {
	delay(500);

	Serial.begin(UART_BAUD1, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);
	serialMcu.begin(UART_BAUD0, SERIAL_PARAM0, SERIAL0_RXPIN, SERIAL0_TXPIN);

	Serial.println();
	//------- server connection  preferences ---------------
	preferences.begin("Punch", false);
	ssid = preferences.getString("ssid", DEF_SSID);
	password = preferences.getString("password", DEF_PASSWORD);
	ipServer = preferences.getLong("ipServer", DEF_IPSERVER);
	portServer = preferences.getLong("portServer", DEF_PORTSERVER);
	enableWifi  = preferences.getBool("enableWifi", DEF_ENABLEWIFI);
	Serial.printf("Setup::Enable Wifi = %d\r\n", enableWifi);
	preferences.end();
	//------------------------------
	serialMcu.flush();
	//Explain ESP2 wakeup reason
	print_wakeup_reason();
	if(!enableWifi){
		Serial.println("Wifi is disabled.");
		Serial.println("...accepting messages from main mcu...");
		return;
	}

	// Try connect WiFi network
	WiFi.begin(ssid.c_str(), password.c_str());
	Serial.println("");
	Serial.printf("Connecting to wifi: ssid: %s  password: %s \r\n", ssid.c_str(), password.c_str());
	int count_try = 0;
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
		count_try++;    // Increase try counter
		if (count_try >= wifi_max_try) {
			// Sorry but wifi is not available 
			Serial.println("\n");
			Serial.println("Impossible to establish WiFi connection.");

			print_wifi_error();
			Serial.println("Sleep a little and retry later, bye.");

			// Set the wakeup 
			esp_sleep_enable_timer_wakeup(duration_deep_sleep * 1000000);
			// And ask processor to sleep now 
			esp_deep_sleep_start();
		}
	}
	Serial.println();
	Serial.printf("Connected to network, my IP address: "); Serial.println(WiFi.localIP());

	Serial.print("Connecting to server: ip="); Serial.print(ipServer);
	Serial.printf(" port=%d\r\n", portServer);
	if (!client.connect(ipServer, portServer, 12000)) {
		Serial.println("Connection to server failed.");
		Serial.println("...accepting messages from main mcu(USB)...");
		delay(1000);
		return;
	}

	Serial.println("Connect as client ok.");
	Serial.println("======================================================");
	Serial.println("Test COM1 or Serial.println: Data Transfer");
	Serial.println("** This terminal act like a Punch MCU talking to Wifi module **");
	Serial.println("Terminal (MCU) <---> WifiModule/Brigde <---> Wifi Client");
	Serial.println("===== Waiting for Wifi/TCP connection/data <--> =====");
	Serial.println("Punch UART  <--> Wifi");
	//esp_err_t esp_wifi_set_max_tx_power(50);  //lower WiFi Power

  //====================== OTA setup ======================================
#ifdef OTA_HANDLER
	ArduinoOTA
		.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
			})
		.onEnd([]() {
				Serial.println("\nEnd");
			})
				.onProgress([](unsigned int progress, unsigned int total) {
				Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
					})
				.onError([](ota_error_t error) {
						Serial.printf("Error[%u]: ", error);
						if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
						else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
						else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
						else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
						else if (error == OTA_END_ERROR) Serial.println("End Failed");
					});
					// if DNSServer is started with "*" for domain name, it will reply with
					// provided IP to all DNS request

					ArduinoOTA.begin();
#endif // OTA_HANDLER    
}

//=========================================================================

bool ExecMessage(uint8_t* pMsg)
{
	if (pMsg[0] != 0xC0) return false;
	Serial.printf("ExecMessage: data size %u\r\n",(uint8_t)pMsg[2]);
	switch (pMsg[1]) {
	//all setting require a reset/reboot **************
	case ID_WIFI_SET_REMOTE_AP_SSID:
		pMsg[3+30]=0;
		ssid = (char*)&pMsg[3];	
		//send response to mcu
		pMsg[0]=RESPONSE;
		pMsg[2]=0;
		//serialMcu.write(pMsg, 3+pMsg[2]); // now send to UART for Wifi :
		preferences.begin("Punch", false);
		preferences.putString("ssid", ssid);
		preferences.end();
		//
		Serial.printf("   SET_AP_SSID: %s\r\n",ssid.c_str());
		break;
	case ID_WIFI_SET_REMOTE_AP_PW :
		pMsg[3+25]=0;
		password = (char*)&pMsg[3];	
		//send response to mcu
		pMsg[0]=RESPONSE;
		pMsg[2]=0;
		//serialMcu.write(pMsg, 3+pMsg[2]); // now send to UART for Wifi :
		preferences.begin("Punch", false);
		preferences.putString("password", password.c_str());
		preferences.end();
		//
		Serial.printf("   SET_AP_PW: %s\r\n",password.c_str());
		break;
	case ID_WIFI_SET_IP_PORT:
		ipServer = *(uint32_t*)&pMsg[3];
		portServer = *(uint16_t*)&pMsg[7];
		//send response to mcu
		pMsg[0]=RESPONSE;
		pMsg[2]=0;
		//serialMcu.write(pMsg, 3+pMsg[2]); // now send to UART for Wifi :
		preferences.begin("Punch", false);
		preferences.putLong("ipServer", ipServer);
		preferences.putLong("portServer", portServer);
		preferences.end();
		//
		Serial.printf("   SET_IP_PORT: port= %u  ip=",portServer);
		Serial.println(ipServer);
		Serial.println("Please reset the hardawre.");
		break;
	case ID_WIFI_ENABLE:{
		//if(enableWifi == *(uint8_t*)&pMsg[3])
			//break;
		//else can use this msg to reset me.
		enableWifi = *(uint8_t*)&pMsg[3];
		preferences.begin("Punch", false);
		preferences.putBool("enableWifi", enableWifi);
		preferences.end();
		//send response to mcu
		pMsg[0]=RESPONSE;
		pMsg[2]=0;
		//serialMcu.write(pMsg, 3+pMsg[2]); // now send to UART for Wifi :
		preferences.begin("Punch", false);
		Serial.printf("Enable Wifi = %d\r\n", enableWifi);
		preferences.end();
		//
		ESP.restart();
		break;
	}
	case ID_WIFI_RESTART:
		ESP.restart();
		break;
	default:
		break;
	}
	return true;
}

int ixCur = 0;
uint32_t ixTimer=0;
void loop()
{
#ifdef OTA_HANDLER
	ArduinoOTA.handle();
#endif // OTA_HANDLER
	//===== UART <-- Wifie TCP ===========
	// COM[0](MCU), COM[1] (debug output)
	//=== Wifi(Punch App) ---> MCU UART0 ===
	while (client.available())
	{
		buf[nBuf] = client.read(); // read char from TCP Wifi client
		if (nBuf < bufferSize - 1)
			nBuf++;
	}
	//---------------------------------------------------------
	serialMcu.write(buf, nBuf); // now send to UART for Wifi :
	nBuf = 0;
	//
	//=== MCU UART0 ---> Wifi(Punch App)==========================
	//
	while (serialMcu.available())
	{
		buf[nBuf] = serialMcu.read(); // read char from UART
		if (nBuf < bufferSize - 1) nBuf++;
	}
	//---- intercept and check for Punch message ---------
	if ((msgsize + nBuf) > 64){//something is wrong!
		Serial.println("msgsize>64");
		memset(arMsg,0,sizeof(arMsg));
		msgsize = 0;
	}
	if (nBuf > 0) {
		memcpy(&arMsg[msgsize], buf, nBuf);
		msgsize += nBuf;
	}
	if (msgsize > 0 && arMsg[0] == 0xC0) {
		Serial.printf("1:got cmd 0xC0 msgsize=%d\r\n",msgsize);
		if (msgsize > 1){
			if((arMsg[1] == ID_WIFI_SET_REMOTE_AP_SSID || arMsg[1] == ID_WIFI_SET_REMOTE_AP_PW ||
							arMsg[1] == ID_WIFI_SET_IP_PORT || arMsg[1] == ID_WIFI_ENABLE)) {
				if (msgsize > 2 && msgsize >= arMsg[2] + 3) {
					Serial.printf("3:Got Wifi msgsize=%d MsgId=0x%x data=%u count=%d\r\n",msgsize,arMsg[1],arMsg[2],ixCur++);
					ExecMessage(arMsg);
					memset(arMsg,0,sizeof(arMsg));
					msgsize = 0;
					serialMcu.flush();
					return;//do not pass on to wifi
				}
				//get next byte until arMsg[2]==datasize
				Serial.printf("2:got msgid msgsize=%d...wait for data\r\n",msgsize);
				return;//do not pass on to wifi
			}
			else {
				msgsize=0;
				memset(arMsg,0,sizeof(arMsg));
				Serial.println("4: bad or unknown msg");
				serialMcu.flush();
				return;
			}
		}
		//else	msgsize=0;
		Serial.println("5:wait for data");
		delay(10);
		if(ixTimer++ > 10){//reset msg
			memset(arMsg,0,sizeof(arMsg));
			msgsize = 0;
			ixTimer = 0;
			serialMcu.flush();
			Serial.println("recv msg timeout - reset");
		}
		return;//do not pass on to wifi
	}
	else msgsize = 0;
	//----------------------
	client.write(buf, nBuf); // now send to TCP WiFi client
	nBuf = 0;

}
