#include "config.h"  //contain contants for networke connection (see constants-tamplates.h)
#include <ESP8266WiFi.h> //conatins Wifi classes
#include <ESP8266HTTPClient.h> //make a http request
#include "DHT.h" //communicatin with dht11 module

//DHT
#define DHTPIN 0     
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//sleepsetup
#define SLEEPTIME 10 //sleep for 10 minutes

//other constants
#define SERIALOUTPUT

void setup() {
   #ifdef SERIALOUTPUT
        Serial.begin(115200);
    #endif

    //dht setup
    dht.begin();
}

//connect to wifi configuration in the config.h file
void wifiConnect(){
    #ifdef SERIALOUTPUT
        //estabish Wifi connnection
        Serial.print("Connecting to: ");
        Serial.println(SSIDNAME);
    #endif
        WiFi.mode(WIFI_STA);
        WiFi.begin(SSIDNAME, SSIDPWD);
        int i = 0;
        while (WiFi.status() != WL_CONNECTED && i++<20) {
            delay(500);
            Serial.print(".");
        }//wait for wifi connection but mus 10 seconds
    #ifdef SERIALOUTPUT
        if(WiFi.status() == WL_CONNECTED){

            Serial.println("WiFi connected");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.print("MAC: ");
            Serial.println(WiFi.macAddress());
        }else{
            Serial.println("can't connect to network");
        }
    #endif
}

//disconnect from wifi
void wifiDisconnect(){
    WiFi.disconnect(); 
}

void loop() {
  // put your main code here, to run repeatedly:

}
