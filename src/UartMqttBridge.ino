//https://icircuit.net/arduino-getting-started-mqtt-using-esp32/2138
#include <WiFi.h>
#include <PubSubClient.h>
#include "Network.h"


// Update these with values suitable for your network.
const char* ssid = "FlyFly";
const char* password = "flyuntildie";
//const char* mqtt_server = "iot.eclipse.org";
const char* mqtt_server = "203.150.107.106";
#define mqtt_port 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_SERIAL_PUBLISH_CH "CASSAVA-DEMO-01/rx"
#define MQTT_SERIAL_RECEIVER_CH "CASSAVA-DEMO-01/tx"

//WiFiClient wifiClient;

PubSubClient pubsubClient(*Net.GetClient());


void mqtt_connect() {
  // Loop until we're reconnected
  while (!pubsubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (pubsubClient.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      pubsubClient.publish("/post-it", "hello world");
      // ... and resubscribe
      pubsubClient.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubsubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for
  Net.PowerOn();
  Net.Connect();
 
  pubsubClient.setServer(mqtt_server, mqtt_port);
  pubsubClient.setCallback(callback);
  mqtt_connect();
}


void publishSerialData(char *serialData){
  if (!pubsubClient.connected()) {
    mqtt_connect();
  }
  pubsubClient.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}

int loop_cnt=0;
void loop() {


    //Net.PowerOn();
    //Net.Connect();
    //Net.HttpGet();
   pubsubClient.loop();
   if (Serial.available() > 0) {
     char bfr[501];
     memset(bfr,0, 501);
     Serial.readBytesUntil( '\n',bfr,500);
     publishSerialData(bfr);
   }
   char msg[20] ;
   sprintf(msg,"test %ld\r\n",loop_cnt);
   if(loop_cnt++%500000 == 0) publishSerialData(msg);
    //Net.Disconnect();
    //Net.PowerOff();

 }