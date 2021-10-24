// config: ////////////////////////////////////////////////////////////
// 
//#define BLUETOOTH
#define OTA_HANDLER 
//#define MODE_AP // phone connects directly to ESP
#define PROTOCOL_TCP

bool debug = true;

#define VERSION "1.00"

#define DEF_SSID    "eMedia Net_EXT" //host wifi AP
#define DEF_PASSWORD "thanhtruong"//host wifi AP
#define DEF_IPSERVER IPAddress(192, 168, 1, 10) //host server
#define DEF_PORTSERVER 9500
#define DEF_ENABLEWIFI true
#define DEF_CONNECT_TIMEOUT	20000 //connect to server timeout. todo - change to longer for release
IPAddress netmask(255, 255, 255, 0);
String ssid = DEF_SSID; //host wifi
String password = DEF_PASSWORD;//host wifi
IPAddress ipServer = DEF_IPSERVER;//host wifi
uint16_t portServer =  DEF_PORTSERVER ;
bool enableWifi = DEF_ENABLEWIFI;
bool wasconnected = false;
bool profiling = false;//todo main can send a msg for this flag.

#define WCOMMAND 	0xC1		//command from f1023rb mcu
#define WRESPONSE 	0xD1		//response to f103rb mcu
// message from mcu ----------------------
#define	ID_WIFI_SET_REMOTE_AP_SSID 	0x21 //set 30 bytes ssid, including NULL
#define	ID_WIFI_GET_REMOTE_AP_SSID 	0x22//return 30 bytes ssid, including NULL
#define	ID_WIFI_SET_REMOTE_AP_PW 	0x23	//set 20 bytes password including NULL
#define	ID_WIFI_GET_REMOTE_AP_PW 	0x24	//return 20 bytes password including NULL
#define	ID_WIFI_SET_IP_PORT  		0x25 	//remote access point (PC-tcp socker server): ip and port
										//set 4 bytes ipAddr, 2 byte port#
#define ID_WIFI_GET_IP_PORT  		0x26		//return 4 bytes ipAddr, 2 byte port#
// 
#define ID_WIFI_ENABLE				0x27 //disable wifi, just use usart to communicate with main mcu for messaging.
#define ID_WIFI_PROFILING			0x28 //todo - mcu set the profiling flag
#define ID_WIFI_RESTART				0x29 //reset me - not use yet
/*************************  COM Port 0 *******************************/
// Doit Kit:COM2<-->CP210x (jumper (pc COM5)):tobe connect to MCU ---
#define UART_BAUD0 115200           // Baudrate UART2
#define SERIAL_PARAM0 SERIAL_8N1    // Data/Parity/Stop UART2
#define SERIAL0_RXPIN 16            // receive Pin UART2
#define SERIAL0_TXPIN 17            // transmit Pin UART2
/*************************  COM Port 1 *******************************/
// Doit Kit:USB<-->CP210x (on board mini USB (pc COM8)) -----------
#define UART_BAUD1 115200           // Baudrate UART0
#define SERIAL_PARAM1 SERIAL_8N1    // Data/Parity/Stop UART0
#define SERIAL1_RXPIN 3             // receive Pin UART0
#define SERIAL1_TXPIN 1             // transmit Pin UART0
//--------------------------
#define bufferSize 64

//////////////////////////////////////////////////////////////////////////
