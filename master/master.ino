#include "config.h"  //contain contants for networke connection (see constants-tamplates.h)
#include <ESP8266WiFi.h> //conatins Wifi classes
#include <ESP8266HTTPClient.h> //make a http request
#include <SoftwareSerial.h> //to use a second serial port

//shiftregister
#define DATAPIN 13
#define STORAGEPIN 14
#define SHIFTPIN 12

//display values
#define EMPTY 0x00      //all LEDs off
#define RED 0x01	    //led RED at pin 0 on
#define YELLOW 0x02     //led YELLOW at pin 1 on
#define GREEN 0x04	    //led GREEN at pin 2 on
#define BLUE 0x08       //led YELLOW at pin 3 on
#define HEATER 0x10	    //turn on heater for the gas sensor at pin 4

//sleepsetup
#define SLEEPTIME 10 //sleep for 10 minutes
#define WAITHEATTIME 30 //wait for 30 seconds

//Serial constants
#define NOREQUEST -1
#define TEMPERATURE 0
#define HUMIDITY 1
#define AIRQUALITY 2

// RX, TX 
SoftwareSerial softSer(5, 4, false, 8);

//other constants
#define SERIALOUTPUT 1 //culd cause problems with dht11

union serialfloat {
    char cval[4];
    float fval;
} sf;

union serialint {
    char cval[4];
    int ival;
} si;

void setup() {
    #ifdef SERIALOUTPUT
        Serial.begin(115200);
    #endif
    softSer.begin(300);

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
    #ifdef SERIALOUTPUT
      Serial.println(httpRequest);
    #endif
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

float seralRequestData(int value){
    #ifdef SERIALOUTPUT
      Serial.print("\n-----------------------------\nsend: ");
    #endif
    si.ival = value;
    #ifdef SERIALOUTPUT
      Serial.print(si.ival);
      Serial.print("(");
      Serial.print(si.cval[0],DEC);
      Serial.print(",");
      Serial.print(si.cval[1],DEC);
      Serial.println(")");
    #endif
    softSer.write(si.cval,4);   //send dada
    //wait until data ar send back
    while(!softSer.available()){
        #ifdef SERIALOUTPUT
          Serial.println("wait for answere ...");
        #endif
        delay(1000);
    }
    //read the receaved data
    sf.fval=-100.0;
    if (softSer.available()) {
        #ifdef SERIALOUTPUT
            Serial.println("received");
        #endif
        int i = 0;
        while(softSer.available() && i<4) {
            sf.cval[i++] = softSer.read();
        }
    }
    #ifdef SERIALOUTPUT
      Serial.println(sf.fval);
      for(int i=0;i<4;i++){
          Serial.print(sf.cval[i],DEC);
          Serial.print(" ");
      }
      Serial.println("-----------------------------------");
    #endif
    return sf.fval;
}

void goToBed(float timeToSleep){
    #ifdef SERIALOUTPUT
        Serial.print("sleep: ");
        Serial.println(timeToSleep);
    #endif
    ESP.deepSleep(timeToSleep*60*1000000); // sleep SLEEPTIME minutes
}

void loop() {
    softSer.listen();
    display(EMPTY);
    display(HEATER);

    delay(WAITHEATTIME*1000);

    //connect to wifi
    wifiConnect();

    //get sensor data
    float temperature, humidity, airPPM;

    temperature = seralRequestData(TEMPERATURE);
    humidity = seralRequestData(HUMIDITY);
    airPPM = seralRequestData(AIRQUALITY);

    display(EMPTY);
    
    //check values
    if (isnan(temperature) || isnan(humidity)) {
        #ifdef SERIALOUTPUT     
            Serial.println("Error while reading temperature and humidity!");
        #endif
        display(RED);
    }else{
        int status = sendRequest(temperature,humidity,airPPM);
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
    goToBed(SLEEPTIME);

}
