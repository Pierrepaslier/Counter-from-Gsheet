/*---Ooho Counter from Google Sheet Mk.1
**---Nodemcu Lua Lolin Module V3 ESP8266 board: https://www.amazon.co.uk/gp/product/B06Y1ZPNMS/ref=oh_aui_detailpage_o00_s01?ie=UTF8&psc=1 
**---MAX7219 8x32 4-in-1 Dot Matrix MCU LED Display Module https://www.amazon.co.uk/gp/product/B07HJDV3HN/ref=oh_aui_detailpage_o00_s01?ie=UTF8&psc=1
**---Last Update: 9/11/2018 by Pierre-Yves Paslier_ creation */

/*
  NodeMCU pins    -> EasyMatrix pins
  MOSI-D7-GPIO13  -> DIN //
  CLK-D5-GPIO14   -> Clk
  GPIO0-D3        -> LOAD
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

#include "HTTPSRedirect.h"
#include "DebugMacros.h"
#include <Wire.h>

// for stack analytics
extern "C" {
#include <cont.h>
  extern cont_t g_cont;
}

// Fill ssid and password with your network credentials
const char* ssid = "****";
const char* password = "*******";

const char* host = "script.google.com";
// Replace with your own script id to make server side changes
const char *GScriptId = "********";

const int httpsPort = 443;

// echo | openssl s_client -connect script.google.com:443 |& openssl x509 -fingerprint -noout
const char* fingerprint = "";

// Write to Google Spreadsheet
String url = String("/macros/s/") + GScriptId + "/exec?value";
// Fetch Google Calendar events for 1 week ahead
String url2 = String("/macros/s/") + GScriptId + "/exec?cal";
// Read from Google Spreadsheet
String url3 = String("/macros/s/") + GScriptId + "/exec?read";

String str;

String payload_base =  "{\"command\": \"appendRow\", \
                    \"sheet_name\": \"Sheet1\", \
                    \"values\": ";
String payload = "";

HTTPSRedirect* client = nullptr;
// used to store the values of free stack and heap
// before the HTTPSRedirect object is instantiated
// so that they can be written to Google sheets
// upon instantiation



// ******************* String form to sent to the client-browser ************************************
String form =
  "<p>"
  "<center>"
  "<h1>ESP8266 Web Server</h1>"
  "<form action='msg'><p>Type your message <input type='text' name='msg' size=100 autofocus> <input type='submit' value='Submit'></form>"
  "</center>";

ESP8266WebServer server(80);                             // HTTP server will listen at port 80
long period;
int offset = 1, refresh = 0;
int pinCS = 0; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 8;
int numberOfVerticalDisplays = 1;
String decodedMsg;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

String tape = "Arduino";
int wait = 75; // In milliseconds

int spacer = 2;
int width = 5 + spacer; // The font width is 5 pixels

unsigned long delayBetweenChecks = 5000; //mean time between api requests
unsigned long whenDueToCheck = 0;

/*
  handles the messages coming from the webbrowser, restores a few special characters and
  constructs the strings that can be sent to the oled display
*/
void handle_msg() {

  matrix.fillScreen(LOW);
  server.send(200, "text/html", form);    // Send same page so they can send another msg
  refresh = 1;
  // Display msg on Oled
  String msg =  str;
  int length = msg.length();
  msg[length - 1] = '\0'; //Remove weird character at end of string
  msg[length - 2] = '\0';

  Serial.println(msg);
  decodedMsg = msg;
  // Restore special characters that are misformed to %char by the client browser
  decodedMsg.replace("+", " ");
  decodedMsg.replace("%21", "!");
  decodedMsg.replace("%22", "");
  decodedMsg.replace("%23", "#");
  decodedMsg.replace("%24", "$");
  decodedMsg.replace("%25", "%");
  decodedMsg.replace("%26", "&");
  decodedMsg.replace("%27", "'");
  decodedMsg.replace("%28", "(");
  decodedMsg.replace("%29", ")");
  decodedMsg.replace("%2A", "*");
  decodedMsg.replace("%2B", "+");
  decodedMsg.replace("%2C", ",");
  decodedMsg.replace("%2F", "/");
  decodedMsg.replace("%3A", ":");
  decodedMsg.replace("%3B", ";");
  decodedMsg.replace("%3C", "<");
  decodedMsg.replace("%3D", "=");
  decodedMsg.replace("%3E", ">");
  decodedMsg.replace("%3F", "?");
  decodedMsg.replace("%40", "@");
  //Serial.println(decodedMsg);                   // print original string to monitor



  //Serial.println(' ');                          // new line in monitor
}

void setup(void) {
  matrix.setIntensity(10); // Use a value between 0 and 15 for brightness

  // Adjust to your own needs
  //  matrix.setPosition(0, 1, 0); // The first display is at <0, 0>
  //  matrix.setPosition(1, 0, 0); // The second display is at <1, 0>

  // Adjust to your own needs
  matrix.setRotation(0, 3);
  matrix.setRotation(1, 3);
  matrix.setRotation(2, 3);
  matrix.setRotation(3, 3);  
  matrix.setRotation(4, 3);
  matrix.setRotation(5, 3);
  matrix.setRotation(6, 3);
  matrix.setRotation(7, 3);

  matrix.setPosition(0, 7, 0); // The first display is at <0, 7>
  matrix.setPosition(1, 6, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 5, 0); // The third display is at <2, 0>
  matrix.setPosition(3, 4, 0); // And the last display is at <3, 0>
  matrix.setPosition(4, 3, 0); // The first display is at <0, 0>
  matrix.setPosition(5, 2, 0); // The second display is at <1, 0>
  matrix.setPosition(6, 1, 0); // The third display is at <2, 0>
  matrix.setPosition(7, 0, 0); // And the last display is at <3, 0>

  //ESP.wdtDisable();                               // used to debug, disable wachdog timer,

  Serial.begin(115200);
  Serial.flush();



  //Establish Wifi Connection
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  // flush() is needed to print the above (connecting...) message reliably,
  // in case the wireless connection doesn't go through
  Serial.flush();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());



  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  Serial.print("Connected to ");
  Serial.println(host);


  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }

  if (client->verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }

  // Note: setup() must finish within approx. 1s, or the the watchdog timer
  // will reset the chip. Hence don't put too many requests in setup()
  // ref: https://github.com/esp8266/Arduino/issues/34

  Serial.println("\nGET: Write into cell 'A1'");
  Serial.println("=========================");

  // fetch spreadsheet data
  //client->GET(url, host);

  // fetch spreadsheet data
  // client->GET(url2, host);

  // fetch spreadsheet data
  client->GET(url3, host);
}


void loop(void) {
  unsigned long timeNow = millis();
  if ((timeNow > whenDueToCheck))  {
    //Do google stuff

    static int error_count = 0;
    static int connect_count = 0;
    const unsigned int MAX_CONNECT = 20;
    static bool flag = false;

    if (client->GET(url3, host)) {
      ++connect_count;
      str = client->getResponseBody();
      Serial.println(str);
    }
    else {
      ++error_count;
      DPRINT("Error-count while connecting: ");
      DPRINTLN(error_count);
    }

    // In my testing on a ESP-01, a delay of less than 1500 resulted
    // in a crash and reboot after about 50 loop runs.
    delay(1000);
    // End of google stuff


    for ( int j = 0 ; j < 2 ; j++ ) {
      handle_msg();
      for ( int i = 0 ; i < width * decodedMsg.length() + matrix.width() - 1 - spacer; i++ ) {
        //server.handleClient();                        // checks for incoming messages
        if (refresh == 1) i = 0;
        refresh = 0;
        matrix.fillScreen(LOW);

        int letter = i / width;
        int x = (matrix.width() - 1) - i % width;
        int y = (matrix.height() - 8) / 2; // center the text vertically

        while ( x + width - spacer >= 0 && letter >= 0 ) {
          if ( letter < decodedMsg.length() ) {
            matrix.drawChar(x, y, decodedMsg[letter], HIGH, LOW, 1);
          }

          letter--;
          x -= width;
        }

        matrix.write(); // Send bitmap to display

        delay(wait);
      }
      Serial.println(j);
    }
    whenDueToCheck = timeNow + delayBetweenChecks;
  }

}



