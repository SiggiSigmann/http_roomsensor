#include "DHT.h" //communicatin with dht11 module
#include "MQ135.h" //interprate mq135 analog output

//DHT
#define DHTPIN 0     
#define DHTTYPE DHT11
DHT dht = DHT(DHTPIN, DHTTYPE);

//MQ135
#define MQ135PIN A0
MQ135 gasSensor = MQ135(MQ135PIN); 
