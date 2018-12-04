#include "DHT.h" //communicatin with dht11 module
#include "MQ135.h" //interprate mq135 analog output
#include <Wire.h> //library for I2C communication
#include <SoftwareSerial.h>

// software serial #1: RX = digital pin 10, TX = digital pin 11
SoftwareSerial softSer(10, 11);

//DHT
#define DHTPIN 2     
#define DHTTYPE DHT11
DHT dht = DHT(DHTPIN, DHTTYPE);

//MQ135
#define MQ135PIN A0
MQ135 gasSensor = MQ135(MQ135PIN); 

//other constants
#define SERIALOUTPUT 1 //culd cause problems with dht11

//variable definition
//-1 = non
//
char requestedValue = -1;
union u_tag {
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

    //dht setup
    dht.begin();
}

//write temperature and humiditi in given variables
//-1 error
float getTemp(){
    //read values form sensore
    float t = dht.readTemperature();

    #ifdef SERIALOUTPUT
        Serial.print("Temperatur: ");
        Serial.println(t);
    #endif
    return t;
}

//write temperature and humiditi in given variables
//-1 error
float getHum(){
    //read values form sensore
    float h = dht.readHumidity();

    #ifdef SERIALOUTPUT
        Serial.print("Humidity: ");
        Serial.println(h);
    #endif
    return h;
}

//write air quality (in ppm) in given variable
float getAirPPM(){
    //measure air
    float a = gasSensor.getPPM();

    #ifdef SERIALOUTPUT
        Serial.print("AirQuality: ");
        Serial.println(a);
    #endif
    return a;
}

void waitForData(){
    while(!softSer.available()){
        delay(1000);
    }
}
void receivedData(){
  Serial.print("how many: ");
  Serial.println(softSer.available());
    if (softSer.available()) {
        int i = 0;
        while(softSer.available() && i<4) {
            si.cval[i] = softSer.read();
            Serial.print(si.cval[i],DEC);
            Serial.print(" ");
            i++;
        }
        Serial.print("\nreceived: ");
        Serial.print(si.ival);
    Serial.print("(");
    Serial.print(si.cval[0],DEC);
    Serial.print(",");
    Serial.print(si.cval[1],DEC);
    Serial.println(")");
        softSer.flush();
        
    }
}

void loop(){
  Serial.println("----------");
    softSer.listen();

    //wait and receive data
    waitForData();
    receivedData();

    //get measuermentvalues
    sf.fval = -100.0;   //defaultvalue
    Serial.print("send value ");
    switch (si.ival) {
        case 0:
            sf.fval = getTemp();
            Serial.print("getTemp() = ");
            break;
        case 1:
            sf.fval = getHum();
            Serial.print("getHum() = ");
            break;
        case 2:
            sf.fval = getAirPPM();
            Serial.print("getAirPPM() = ");
            break;
    }
    Serial.println(sf.fval);

    softSer.write(sf.cval,4); //send requested Data back
}
