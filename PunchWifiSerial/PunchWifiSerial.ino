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

HardwareSerial Serial_one(1);
HardwareSerial Serial_two(2);
HardwareSerial* COM[NUM_COM] = {&Serial, &Serial_one , &Serial_two};

#define MAX_NMEA_CLIENTS 4
#include <WiFiClient.h>
WiFiServer server_0(SERIAL0_TCP_PORT);
WiFiServer server_1(SERIAL1_TCP_PORT);
WiFiServer server_2(SERIAL2_TCP_PORT);
WiFiServer *server[NUM_COM] = {&server_0, &server_1, &server_2};
WiFiClient TCPClient[NUM_COM][MAX_NMEA_CLIENTS];



uint8_t buf1[NUM_COM][bufferSize];
uint16_t i1[NUM_COM] = {0, 0, 0};

uint8_t buf2[NUM_COM][bufferSize];
uint16_t i2[NUM_COM] = {0, 0, 0};

uint8_t BTbuf[bufferSize];
uint16_t iBT = 0;


void setup() {

  delay(500);

  COM[0]->begin(UART_BAUD0, SERIAL_PARAM0, SERIAL0_RXPIN, SERIAL0_TXPIN);
  COM[1]->begin(UART_BAUD1, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);
  COM[2]->begin(UART_BAUD2, SERIAL_PARAM2, SERIAL2_RXPIN, SERIAL2_TXPIN);

  if (debug) COM[DEBUG_COM]->println("\n\nPunch WiFi serial bridge V1.00");
#ifdef MODE_AP
  if (debug) COM[DEBUG_COM]->println("Open ESP Access Point mode");
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

  COM[0]->println("Starting TCP Server 0 - COM0");
  if (debug)
    COM[DEBUG_COM]->println("Starting TCP Server 0 - COM0 printo COM_Debug");
  server[0]->begin(); // start TCP server
  server[0]->setNoDelay(true);
  COM[1]->println("Starting TCP Server 1 - COM1");
  if (debug)
    COM[DEBUG_COM]->println("Starting TCP Server 1 - COM1 printo COM_Debug");
  server[1]->begin(); // start TCP server
  server[1]->setNoDelay(true);
  COM[2]->println("Starting TCP Server 2 - COM2");
  if (debug)
    COM[DEBUG_COM]->println("Starting TCP Server 2 - COM2 printo COM_Debug");
  server[2]->begin(); // start TCP server
  server[2]->setNoDelay(true);

  esp_err_t esp_wifi_set_max_tx_power(50);  //lower WiFi Power
}


void loop()
{
#ifdef OTA_HANDLER
  ArduinoOTA.handle();
#endif // OTA_HANDLER

  for (int num = 0; num < NUM_COM ; num++)
  {
    if (server[num]->hasClient())
    {
      for (byte i = 0; i < MAX_NMEA_CLIENTS; i++) {
        //find free/disconnected spot
        if (!TCPClient[num][i] || !TCPClient[num][i].connected()) {
          if (TCPClient[num][i])
            TCPClient[num][i].stop();
          TCPClient[num][i] = server[num]->available();
          if (debug) COM[DEBUG_COM]->print("New client for COM");
          if (debug) COM[DEBUG_COM]->print(num);
          if (debug) COM[DEBUG_COM]->println(i);
          continue;
        }
      }
      //no free/disconnected spot so reject
      WiFiClient TmpserverClient = server[num]->available();
      TmpserverClient.stop();
    }
  }

  for (int num = 0; num < NUM_COM ; num++)
  {
    if (COM[num] != NULL)
    {
      for (byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
      {
        if (TCPClient[num][cln])
        {
          while (TCPClient[num][cln].available())
          {
            buf1[num][i1[num]] = TCPClient[num][cln].read(); // read char from client (LK8000 app)
            if (i1[num] < bufferSize - 1) i1[num]++;
          }

          COM[num]->write(buf1[num], i1[num]); // now send to UART(num):
          i1[num] = 0;
        }
      }

      if (COM[num]->available())
      {
        while (COM[num]->available())
        {
          buf2[num][i2[num]] = COM[num]->read(); // read char from UART(num)
          if (i2[num] < bufferSize - 1) i2[num]++;
        }
        // now send to WiFi:
        for (byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
        {
          if (TCPClient[num][cln])
            TCPClient[num][cln].write(buf2[num], i2[num]);
        }
        i2[num] = 0;
      }
    }
  }
}
