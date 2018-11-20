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

//write temperature and humiditi in given variables
//-1 error
int getTempHum(float* temperature, float* humidity){
    //read values form sensore
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    //check values
    if (isnan(h) || isnan(t)) {
    #ifdef SERIALOUTPUT     
            Serial.println("Fehler beim auslesen des Sensors!");
    #endif
        return -1;
    }

    #ifdef SERIALOUTPUT
        Serial.print("Temperatur: ");
        Serial.println(t);
    #endif
    *temperature = t;
    #ifdef SERIALOUTPUT
        Serial.print("Humidity: ");
        Serial.println(h);
    #endif
    *humidity = h;
    return 0;
}

void loop() {

    //connect to wifi
    wifiConnect();

    //get sensor data
    float temperature, humidity;
    int status = getTempHum(&temperature, &humidity);
    if(status != 0){
        #ifdef SERIALOUTPUT
                Serial.print("getTempHum status: ");
                Serial.println(status);
        #endif
    }

    //send request
    if(!status){
        status = sendRequest(temperature,humidity);
        if(status != 0){
            #ifdef SERIALOUTPUT
                Serial.print("sendRequest status: ");
                Serial.println(status);
            #endif
        }else{
            if(temperature<20.0){
                #ifdef SERIALOUTPUT
                    Serial.println("too cold");
                #endif
            }
        }
    }
    
    //disconect from Wifi
    wifiDisconnect();

    //sleep
    #ifdef SERIALOUTPUT
        Serial.println("sleep");
    #endif
    ESP.deepSleep(SLEEPTIME*60*1000000); // sleep SLEEPTIME minutes
}
