#include <Debug.h>
#include <JSN270.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

#define SSID      "SSID"		// your wifi network SSID
#define KEY       "KEY"		// your wifi network password
#define AUTH       "WPA2" 		// your wifi network security (NONE, WEP, WPA, WPA2)

#define USE_DHCP_IP 1

#if !USE_DHCP_IP
#define MY_IP          "192.168.1.133"
#define SUBNET         "255.255.255.0"
#define GATEWAY        "192.168.1.254"
#endif

#define SERVER_PORT    80
#define PROTOCOL       "TCP"

const int redLedPin =  8;
bool redLedStatus = 0;
const int greenLedPin =  9;
bool greenLedStatus = 0;
const int blueLedPin =  10;
bool blueLedStatus = 0;

SoftwareSerial mySerial(3, 2); // RX, TX
 
JSN270 JSN270(&mySerial);

void setup() {
	char c;

	pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  
	mySerial.begin(9600);
	Serial.begin(9600);

	Serial.println("--------- JSN270 Red/Green LED Example --------");

	// wait for initilization of JSN270
	delay(5000);
	//JSN270.reset();
	delay(1000);

	//JSN270.prompt();
	JSN270.sendCommand("at+ver\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);

#if USE_DHCP_IP
	JSN270.dynamicIP();
#else
	JSN270.staticIP(MY_IP, SUBNET, GATEWAY);
#endif    
    
	if (JSN270.join(SSID, KEY, AUTH)) {
		Serial.println("WiFi connect to " SSID);
	}
	else {
		Serial.println("Failed WiFi connect to " SSID);
		Serial.println("Restart System");

		return;
	}    
	delay(1000);

	JSN270.sendCommand("at+wstat\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);        

	JSN270.sendCommand("at+nstat\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);
}

void loop() {
	if (!JSN270.server(SERVER_PORT, PROTOCOL)) {
		Serial.println("Failed connect ");
		Serial.println("Restart System");
	} else {
		Serial.println("Waiting for connection...");
	}
      
	String currentLine = "";                // make a String to hold incoming data from the client
	int get_http_request = 0;

	while (1) {
		if (mySerial.overflow()) {
			Serial.println("SoftwareSerial overflow!");
		}
   
		if (JSN270.available() > 0) {
			char c = JSN270.read();
			Serial.print(c);

			if (c == '\n') {                    // if the byte is a newline character
				if (currentLine.length() == 0) {
					if (get_http_request) {
						Serial.println("new client");
						//Serial.println("HTTP RESPONSE");
						// Enter data mode
						JSN270.sendCommand("at+exit\r");
						delay(100);
						
						// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
						// and a content-type so the client knows what's coming, then a blank line:
						JSN270.println("HTTP/1.1 200 OK");
						JSN270.println("Content-type:text/html");
						JSN270.println();

						// the content of the HTTP response follows the header:
						JSN270.print("Click <a href=\"/RH\">here</a> to turn the RED LED on<br>");
						JSN270.print("Click <a href=\"/RL\">here</a> to turn the RED LED off<br>");
            JSN270.print("Click <a href=\"/GH\">here</a> to turn the GREEN LED on<br>");
            JSN270.print("Click <a href=\"/GL\">here</a> to turn the GREEN LED off<br>");
            JSN270.print("Click <a href=\"/BH\">here</a> to turn the BLUE LED on<br>");
            JSN270.print("Click <a href=\"/BL\">here</a> to turn the BLUE LED off<br>");

						// The HTTP response ends with another blank line:
						JSN270.println();

						// wait until all http response data sent:
						delay(1000);

						// Enter command mode:
						JSN270.print("+++");
						delay(100);
						
						// break out of the while loop:
						break;
					}
				}
				// if you got a newline, then clear currentLine:
				else {                
					// Check the client request:
					if (currentLine.startsWith("GET / HTTP")) {
						Serial.println("HTTP REQUEST");
						get_http_request = 1;
					}
					else if (currentLine.startsWith("GET /RH")) {
						//Serial.println("GET HIGH");
						get_http_request = 1;
						redLedStatus=1;
					}
         else if (currentLine.startsWith("GET /RL")) {
           //Serial.println("GET HIGH");
            get_http_request = 1;
            redLedStatus=0;
          }
          else if (currentLine.startsWith("GET /GH")) {
            //Serial.println("GET HIGH");
            get_http_request = 1;
            greenLedStatus=1;
          }
					else if (currentLine.startsWith("GET /GL")) {
						//Serial.println("GET LOW");
						get_http_request = 1;
            greenLedStatus=0;
					}
          else if (currentLine.startsWith("GET /BH")) {
           //Serial.println("GET LOW");
            get_http_request = 1;
            blueLedStatus=1;
          }
          else if (currentLine.startsWith("GET /BL")) {
            //Serial.println("GET LOW");
            get_http_request = 1;
            blueLedStatus=0;
          }
					currentLine = "";
				}
			}
			else if (c != '\r') {    // if you got anything else but a carriage return character,
				currentLine += c;      // add it to the end of the currentLine
			}
		}
	}

	digitalWrite(redLedPin, redLedStatus);
  digitalWrite(greenLedPin, greenLedStatus);
  digitalWrite(blueLedPin, blueLedStatus);
            
	// close the connection
	JSN270.sendCommand("at+nclose\r");
	Serial.println("client disonnected");
}
