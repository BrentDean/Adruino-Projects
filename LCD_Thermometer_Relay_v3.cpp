////Arudino Projects - Brent Dean

////Successful Test
////LCD output Temp and Fan Status, Relay activated @ 75F, Temp Threshold 74-75 

////////TESTING: One-button Webserver with Temp/Humidity Output

//// Load Wi-Fi library
#include <WiFi.h>

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

WiFiServer server(80);                              //// Set web server port number to 80

String header;                                      //// Variable to store the HTTP request
String relay_pinState = "off";                      //// Auxiliar variables to store the current output state

const int relay_pin = 9;                            //// Assign output variable to GPIO pin

unsigned long currentTime = millis();               //// Current time
unsigned long previousTime = 0;                     //// Previous time
const long timeoutTime = 2000;                      //// Define timeout time in milliseconds (example: 2000ms = 2s)

////Load DHT
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

////Load Liquid Crystal
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);               //// 0x27 is LCD I2C Address


void setup() {

//// Connect to Wi-Fi network with SSID and password
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
//// Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
 
 dht.begin();                                     //// Initialize the sensor
 lcd.backlight();                                 //// Turn on lcd backlight
 lcd.init();                                      //// Initialize lcd
 pinMode(relay_pin, OUTPUT);                      //// Set relay_pin as Output
 digitalWrite(relay_pin, LOW);                    //// Set relay_pin initial state LOW
}


void loop() {
float Temp = dht.readTemperature(true);//// Print the temperature (true) means F // () mean C
float Humidity = dht.readHumidity();
float Temp_Upper = 75;                            //// Upper temperature threshold
float Temp_Lower = 74;                            //// Lower temperature threshold

   lcd.clear();   
   lcd.setCursor(0,0);                            //// Set the cursor on the first row and first column
   lcd.print("T=");
   lcd.print(Temp);   
   lcd.print(" ");
   lcd.print("H=");
   lcd.print(Humidity);//print the humidity
   lcd.print("%");

   if (Temp > Temp_Upper) {
      Serial.println("Fan is ON");
      digitalWrite(relay_pin, HIGH); // turn on
      lcd.setCursor(0,1);                             //// Set the cursor on the LCD second row and first column
      lcd.print("Fan is ON ");                        //// Print Fan status to LCD (ON)
      delay(10);
      
    } else if (Temp < Temp_Lower) {
      Serial.println("Fan is OFF");
      digitalWrite(relay_pin, LOW);                   //// Normal Open (Relay) is open when below threshold
      lcd.setCursor(0,1);                             //// Set the cursor on the LCD second row and first column
      lcd.print("Fan is OFF ");                       //// Print Fan status to LCD (OFF)
}

WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /9/on") >= 0) {
              Serial.println("Relay on");
              relay_pinState = "on";
              digitalWrite(relay_pin, HIGH);
            } else if (header.indexOf("GET /9/off") >= 0) {
              Serial.println("Relay off");
              relay_pinState = "off";
              digitalWrite(relay_pin, LOW);
            } 
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>poo cat is great.</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 9  
            client.println("<p>GPIO 9 - State " + relay_pinState + "</p>");
            // If the relay_pinState is off, it displays the ON button       
            if (relay_pinState=="off") {
              client.println("<p><a href=\"/9/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/9/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
           client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

   delay(2000);
   lcd.clear();
}
