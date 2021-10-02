// ESP32 WiFi <-> 3x UART Bridge
// by AlphaLima
// modified by Thanh Truong 9/26/2021
//  * remove BlueTooth

// Disclaimer: Don't use  for life support systems
// or any other situations where system failure may affect
// user or environmental safety.

#include "config.h"
#include <esp_wifi.h>
#include <WiFi.h>


#ifdef OTA_HANDLER
#include <ArduinoOTA.h>

#endif // OTA_HANDLER
//*** HardwareSerial Serial; Default auto create to/from wifi
HardwareSerial Serial_1(1); //COM1 - debug output
HardwareSerial* pCOM[NUM_COM] = {&Serial, &Serial_1};

#define MAX_NMEA_CLIENTS 4
#include <WiFiClient.h>
WiFiServer server_0(SERIAL0_TCP_PORT);
WiFiServer server_1(SERIAL1_TCP_PORT);
WiFiServer *pServer[NUM_COM] = {&server_0, &server_1};
WiFiClient TCPClient[NUM_COM][MAX_NMEA_CLIENTS];//1 tcp Server for multiple Client


uint8_t buf1[NUM_COM][bufferSize];
uint16_t i1[NUM_COM] = {0, 0};

uint8_t buf2[NUM_COM][bufferSize];
uint16_t i2[NUM_COM] = {0, 0};


void setup() {

  delay(500);

  pCOM[0]->begin(UART_BAUD0, SERIAL_PARAM0, SERIAL0_RXPIN, SERIAL0_TXPIN);
  pCOM[1]->begin(UART_BAUD1, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);

  if (debug) 
    Serial_1.println("\n\nPunch WiFi serial bridge V1.00");
#ifdef MODE_AP
  if (debug) 
    Serial_1.println("Open ESP Access Point mode");
  //AP mode (phone connects directly to ESP) (no router)
  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, pw); // configure ssid and password for softAP
  delay(2000); // VERY IMPORTANT
  WiFi.softAPConfig(ip, ip, netmask); // configure ip address for softAP

#endif


#ifdef OTA_HANDLER
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial_1.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial_1.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial_1.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial_1.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial_1.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial_1.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial_1.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial_1.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial_1.println("End Failed");
  });
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request

  ArduinoOTA.begin();
#endif // OTA_HANDLER    

  Serial_1.println("Test COM0 or Serial_1.println: Data Transfer");
  Serial_1.println("** This terminal act like a Punch MCU talking to Wifi module **");
  Serial_1.println("Terminal (MCU) <---> WifiModule/Brigde <---> Wifi Client");
  Serial_1.println("===== Waiting for Wifi/TCP connection/data <--> =====");
  Serial_1.println("Punch UART0  <--> Wifi");
  if (debug)
    Serial_1.println("Starting TCP Server 0");
  pServer[0]->begin(); // start TCP server
  pServer[0]->setNoDelay(true);
  Serial_1.println("UART1  Kit's USB port");
  if (debug)
    Serial_1.println("Starting TCP Server 1");
  pServer[1]->begin(); // start TCP server
  pServer[1]->setNoDelay(true);
  //esp_err_t esp_wifi_set_max_tx_power(50);  //lower WiFi Power
}


void loop()
{
#ifdef OTA_HANDLER
  ArduinoOTA.handle();
#endif // OTA_HANDLER

  //======= check TCP client connection =====
  for (int num = 0; num < NUM_COM ; num++)
  {
    if (pServer[num]->hasClient())
    {
      for (byte i = 0; i < MAX_NMEA_CLIENTS; i++) {
        //find free/disconnected spot
        if (!TCPClient[num][i] || !TCPClient[num][i].connected()) {
          if (TCPClient[num][i])
            TCPClient[num][i].stop();
          TCPClient[num][i] = pServer[num]->available();
          if (debug) {
            Serial_1.print("New client for Server ");
            Serial_1.print(num);
            Serial_1.print("  COM ");
            Serial_1.println(i);
          }
          break;//continue;
        }
        else{
          if (debug){
            Serial_1.print("Client slot accupied Server ");
            Serial_1.print(num);
            Serial_1.print("  COM ");
            Serial_1.println(i);
          }
        }
      }
      //no free/disconnected spot so reject
      Serial_1.println("Reject - no slot");
      WiFiClient TmpserverClient = pServer[num]->available();
      TmpserverClient.stop();
    }
  }
  //===== UART <--> Wifie TCP ===========
  // COM[0](MCU), COM[1] (debug output)
  for (int num = 0; num < NUM_COM ; num++)
  {
    if (pCOM[num] != NULL)
    {
      //=== Wifi(Punch App) ---> MCU UART0 ===
      for (byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
      {
        if (TCPClient[num][cln])
        {
          while (TCPClient[num][cln].available())
          {
            buf1[num][i1[num]] = TCPClient[num][cln].read(); // read char from TCP Wifi client
            if (i1[num] < bufferSize - 1) 
              i1[num]++;
          }
          pCOM[num]->write(buf1[num], i1[num]); // now send to UART(num) :
          i1[num] = 0;
        }
      }
      //=== MCU UART0 ---> Wifi(Punch App)===
      if (pCOM[num]->available())
      {
        while (pCOM[num]->available())
        {
          buf2[num][i2[num]] = pCOM[num]->read(); // read char from UART
          if (i2[num] < bufferSize - 1) i2[num]++;
        }
        for (byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
        {
          if (TCPClient[num][cln]) 
            TCPClient[num][cln].write(buf2[num], i2[num]); // now send to TCP WiFi client
        }
        i2[num] = 0;
      }
    }
  }
}
