// config: ////////////////////////////////////////////////////////////
// 
//#define BLUETOOTH
#define OTA_HANDLER 
#define MODE_AP // phone connects directly to ESP


#define PROTOCOL_TCP

bool debug = true;

#define VERSION "1.00"

// For AP mode:
const char *ssid = "Punch";  // You will connect your phone to this Access Point
const char *pw = "YouMe"; // and this is the password
IPAddress ip(192, 168, 5, 1); // 
IPAddress netmask(255, 255, 255, 0);

// You must connect the phone to this AP, then:
// menu -> connect -> Internet(TCP) -> 192.168.5.1:8880  for UART0
//                                  -> 192.168.5.1:8881  for UART1
//                                  -> 192.168.5.1:8882  for UART2





#define NUM_COM   3                 // total number of COM Ports
#define DEBUG_COM 0                 // debug output to COM0
/*************************  COM Port 0 *******************************/
#define UART_BAUD0 115200            // Baudrate UART0
#define SERIAL_PARAM0 SERIAL_8N1    // Data/Parity/Stop UART0
#define SERIAL0_RXPIN 3             // receive Pin UART0
#define SERIAL0_TXPIN 1             // transmit Pin UART0
#define SERIAL0_TCP_PORT 8880       // Wifi Port UART0
/*************************  COM Port 1 *******************************/
                                    // Doit Kit:USB<-->CP210x (on board mini USB) ---
#define UART_BAUD1 115200           // Baudrate UART1 
#define SERIAL_PARAM1 SERIAL_8N1    // Data/Parity/Stop UART1
#define SERIAL1_RXPIN 9             // receive Pin UART1
#define SERIAL1_TXPIN 10            // transmit Pin UART1
#define SERIAL1_TCP_PORT 8881       // Wifi Port UART1
/*************************  COM Port 2 *******************************/
#define UART_BAUD2 115200            // Baudrate UART2
#define SERIAL_PARAM2 SERIAL_8N1    // Data/Parity/Stop UART2
#define SERIAL2_RXPIN 16            // receive Pin UART2
#define SERIAL2_TXPIN 17             // transmit Pin UART2
#define SERIAL2_TCP_PORT 8882       // Wifi Port UART2

/*************************  Default COM Port 2 *******************************/
// todo - may remove all but keep one Server and one UART (on 2)
#define UART_BAUD 115200           // Baudrate UART2
#define SERIAL_PARAM SERIAL_8N1    // Data/Parity/Stop UART2
#define SERIAL_RXPIN 16            // receive Pin UART2
#define SERIAL_TXPIN 17            // transmit Pin UART2
#define SERIAL_TCP_PORT 8885       // Wifi Port UART2

#define bufferSize 1024

//////////////////////////////////////////////////////////////////////////
