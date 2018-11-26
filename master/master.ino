#include "config.h"  //contain contants for networke connection (see constants-tamplates.h)
#include <ESP8266WiFi.h> //conatins Wifi classes
#include <ESP8266HTTPClient.h> //make a http request
#include "DHT.h" //communicatin with dht11 module
#include "MQ135.h" //interprate mq135 analog output

//DHT
#define DHTPIN 0     
#define DHTTYPE DHT11
DHT dht = DHT(DHTPIN, DHTTYPE);

//MQ135
#define MQ135PIN A0
MQ135 gasSensor = MQ135(MQ135PIN); 

//shiftregister
#define DATAPIN 13
#define STORAGEPIN 14
#define SHIFTPIN 12

//display values
#define EMPTY 0x00 //all LEDs off
#define RED 0x01	    //red LED at pin 0 on
#define YELLOW 0x02      //red YELLOW at pin 2 on
#define GREEN 0x04	//red GREEN at pin 1 on
#define BLUE 0x08      //red YELLOW at pin 2 on

//sleepsetup
#define SLEEPTIME 10 //sleep for 10 minutes

//other constants
//#define SERIALOUTPUT 1 //culd cause problems with dht11

void setup() {
   #ifdef SERIALOUTPUT
        Serial.begin(115200);
    #endif

    //dht setup
    dht.begin();

    //shiftregister
    pinMode(SHIFTPIN, OUTPUT);
    pinMode(STORAGEPIN, OUTPUT);
    pinMode(DATAPIN, OUTPUT);
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

//send temperate and humiditi to server via http
//requires a special http server running
//-1 unknown error
//-2 no connection
//-3 no answere
//-4 error while server process http request
int sendRequest(float temperature, float humidity, float airQuality){
    display(BLUE);
    //open connection to HOST at HOSTPORT
    WiFiClient client;
    if (!client.connect(HOST, HOSTPORT)) {
    #ifdef SERIALOUTPUT
            Serial.println("connection failed");
    #endif
            return -2;
        }else{
    #ifdef SERIALOUTPUT
            Serial.println("connected");
    #endif
    }

    //send http request to Host
    String httpRequest = String("GET")+" /?temp="+temperature+"&hum="+humidity + "&air=" + airQuality +
                                " HTTP/1.1\n" +"Host: " + HOST + "\nConnection: close\n\n";
    Serial.println(httpRequest);
    client.print(httpRequest);

    //wait for answere
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 20000) {   //timeout after 20 seconds
        #ifdef SERIALOUTPUT
            Serial.println("no answere form client");
        #endif
            client.stop();
            return -3;
        }
    }

    //get answere
    while (client.available()) {
        String answere = client.readString();
        #ifdef SERIALOUTPUT
            Serial.println(answere);
        #endif
            if(answere.toInt()==-1){
        #ifdef SERIALOUTPUT
                Serial.println("server returend error");
        #endif
            return -4;
        }
    }

    #ifdef SERIALOUTPUT
        Serial.println();
        Serial.println("closing connection");
    #endif
    display(EMPTY);
    return 0;
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
            Serial.println("Error while reading temperature and humidity!");
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

//write air quality (in ppm) in given variable
void getAirPPM(float* airPPM){
    //measure air
    float a = gasSensor.getPPM();

    #ifdef SERIALOUTPUT
        Serial.print("AirQuality: ");
        Serial.println(a);
    #endif
    *airPPM = a;
}

//Displays values with leds
void display(int i){
    #ifdef SERIALOUTPUT
        Serial.print("Display code: ");
        Serial.println(i);
    #endif
    // put your main code here, to run repeatedly:
    digitalWrite(STORAGEPIN, LOW);
    shiftOut(DATAPIN, SHIFTPIN, MSBFIRST, i);
    digitalWrite(STORAGEPIN, HIGH);
}

void loop() {
  display(EMPTY);

    //connect to wifi
    wifiConnect();

    //get sensor data
    float temperature, humidity, airPPM;
    int status = getTempHum(&temperature, &humidity);
    if(status != 0){
        display(RED);
        #ifdef SERIALOUTPUT
                Serial.print("getTempHum status: ");
                Serial.println(status);
        #endif
    }
    getAirPPM(&airPPM);

    //send request
    if(!status){
        status = sendRequest(temperature,humidity,airPPM);
        if(status != 0){
            display(RED);
            #ifdef SERIALOUTPUT
                Serial.print("sendRequest status: ");
                Serial.println(status);
            #endif
        }else{
            display(GREEN);
            delay(1000);
            display(EMPTY);
            if(temperature<20.0){
                display(YELLOW);
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
