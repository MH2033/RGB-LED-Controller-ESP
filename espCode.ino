
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ArduinoJson.h>

#define TIMEOUT 1000
#define READING_DELAY 5

//Control signals between arduino uno and esp8266

//#define RED '0'
//#define BLUE '1'
//#define GREEN '2'
//#define OFF '3'
//#define LED_STATUS '4'
//#define OK '5'
//#define ERR '6'
//#define INTENSITY= 

ESP8266WebServer server(5436);       // Create a webserver object that listens for HTTP request on port 80
ESP8266WiFiGenericClass espSettings; //Create a ESP8266Generic Class that can set ESP8266 settings

time_t currentTime;

String respond;
String macAddr;
String ipAddr;
String deviceInfo;

void setup(void){
 
  Serial.begin(115200);         // Start the Serial communication to send messages to the arduino
  Serial.setTimeout(TIMEOUT);
  
  espSettings.setOutputPower(10);           //Set wifi output power to 10dB

  WiFi.hostname("LED Controller"); //Setting the device hostname
  
  WiFi.mode(WIFI_STA);          //Start in the station mode

  WiFi.begin("MH", "******"); //Access Point credentials
  
  server.on("/deviceInfo", handleInfo);       // Call the 'handleInfo' function when a client requests URI "/handleInfo"
  server.on("/led", handleLed);               // Call the 'handleLed' function when a client requests URI "/led"
  server.onNotFound(handleNotFound);          // When a client requests an unknown URI (i.e. something other than "/led"), call function "handleNotFound"

  server.begin();                           // Actually start the server
}

void loop(void){
  //clearSerial();
  server.handleClient();                    // Listen for HTTP requests from clients
  //clearSerial();
}

void handleInfo(){
  
  if(server.method() == HTTP_GET) {
    macAddr = WiFi.macAddress();     //Save esp8266 mac address in a String
    ipAddr = WiFi.localIP().toString();        //Save esp8266 local ip address in a String
    StaticJsonDocument<200> doc;          //Memory pool for json object

    //Parse the json containing usefull information about the device
    
    doc["deviceName"] = "ESP8266 LED Controller";
    doc["deviceId"] = "ESPLED";
    doc["mac"] = macAddr;
    doc["ip"] = ipAddr;

    serializeJson(doc, deviceInfo);
    server.send(200, "application/json", deviceInfo);
  }
  
  else
    server.send(405, "text/plain", "405: Method Not Allowed");

}

void handleLed() {

  //return LED status
  
  if(server.method() == HTTP_GET){        
    Serial.println("LED_STATUS");

    //currentTime = millis();
    //while(!Serial.available() && (millis() - currentTime) <= TIMEOUT);             //Wait for arduino to respond and leave after timeout

    //delay(READING_DELAY);     //Wait for buffer to fill
    
    respond = Serial.readStringUntil('\n');
      
    if(respond.indexOf("RED") >= 0){
      server.send(200, "application/json", "{\"led\": \"red\"}");
    }
    else if(respond.indexOf("GREEN") >= 0){
      server.send(200, "application/json", "{\"led\": \"green\"}");
    }
    else if(respond.indexOf("BLUE") >= 0){
      server.send(200, "application/json", "{\"led\": \"blue\"}");
    }
    if(respond.indexOf("OFF") >= 0){
      server.send(200, "application/json", "{\"led\": \"red\"}");
    }
    else{
      //Serial.printlnln(respond);
      server.send(500, "text/plain", "Error in reading LED status!");
    }
  }

  //Set LED status
  else if(server.method() == HTTP_POST) {

    //Deserilize JSON object
    StaticJsonDocument<300> JSONBuffer;   //Memory pool for data in the JSON
    deserializeJson(JSONBuffer, server.arg("plain"));

    String intensity = JSONBuffer["intensity"];
    String led = JSONBuffer["led"];
    
    //Send control Commands to arduino
    if(!led.equals("null")){
      //Serial.println(led);
      if(led == "red")
        Serial.println("RED");
         
      else if(led == "green")
        Serial.println("GREEN");
        
      else if(led == "blue")
         Serial.println("BLUE");
        
      else if(led == "off")
        Serial.println("OFF");
  
      //currentTime = millis();
      //while(!Serial.available() && (millis() - currentTime) <= TIMEOUT);           //Wait for arduino to respond and leave after timeout
      
      //delay(READING_DELAY);   //Wait for buffer to fill
      
      respond = Serial.readStringUntil('\n');  
      
      if(respond.indexOf("OK") >= 0){
        server.send(200);
      }

      else {
        server.send(500, "text/plain", "Error in wrtiting LED configurations!");
      }
     }
      
    if(!intensity.equals("null")){
      //Serial.println(intensity);
        Serial.println("INTENSITY=" + intensity);
        
        //currentTime = millis();
        //while(!Serial.available() && (millis() - currentTime) <= TIMEOUT);           //Wait for arduino to respond and leave after timeout
        respond = Serial.readStringUntil('\n'); 
        
        if(respond.indexOf("OK") >= 0){
          server.send(200);
        }
          
        else {
          server.send(500, "text/plain", "Error in wrtiting LED configurations!");
        }
     }
    
    else {
      //Serial.printlnln(respond);
      server.send(500, "text/plain", "Error in wrtiting LED configurations!");
    }
  }
    
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void clearSerial(){
  while(Serial.available()) //Clear serial buffer
      Serial.read();
}
