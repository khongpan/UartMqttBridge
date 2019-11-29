//https://icircuit.net/arduino-getting-started-mqtt-using-esp32/2138
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Network.h"

#define SerialCon Serial2


// Update these with values suitable for your network.
const char* ssid = "FlyFly";
const char* password = "flyuntildie";
//const char* mqtt_server = "iot.eclipse.org";
const char* mqtt_server = "203.150.107.106";
#define mqtt_port 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_SERIAL_PUBLISH_CH "CASSAVA-DEMO-01/tx"
#define MQTT_SERIAL_RECEIVER_CH "CASSAVA-DEMO-01/rx"

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
      pubsubClient.publish("/post-it", "reboot");
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
    //Serial.println("");
    //SerialCon.println("$ rtc get 1");
    SerialCon.write(payload, length);
    //SerialCon.println("");
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for
  SerialCon.begin(38400,SERIAL_8E1,18,23);
  SerialCon.setTimeout(50);

  //Net.Setup();
  Net.PowerOn();
  Net.Connect();
 
  pubsubClient.setServer(mqtt_server, mqtt_port);
  pubsubClient.setCallback(callback);
  mqtt_connect();
  SerialCon.print("\r\n\r\n");
  SerialCon.print("$ sys print_off\r\n");
}


void publishSerialData(char *serialData){
  if (!pubsubClient.connected()) {
    mqtt_connect();
  }
  pubsubClient.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}

void publishSerialData(char *serialData, int len) {
    if (!pubsubClient.connected()) {
    mqtt_connect();
  }
  int i;

  /*
  for(i=0;i<len;i++) {
    if (serialData[len]>128) serialData[len]='#';
  }
  */

  //Serial.print("pub:");
  //Serial.print(serialData);
  pubsubClient.publish(MQTT_SERIAL_PUBLISH_CH, (const uint8_t *)serialData, len);
  
}

const int MAX_LINE=300;
const int BUFF_SIZE=10240;
char buff[BUFF_SIZE];
int line[MAX_LINE+1];
int cur_line=0;
int last_line=0;

void loop() {


  Net.Maintain();
  if (!pubsubClient.connected()) {
    mqtt_connect();
  }
  pubsubClient.loop();

  
  int line_cnt=0;
  int pos,len;
     
  //SerialCon.setTimeout(100); 
  line[0]=0;
  pos=0;  
  len=SerialCon.readBytesUntil('\n',buff+pos,100);
  

  while((len>0) && (line_cnt<MAX_LINE) && (pos < BUFF_SIZE-100)) {
    //Serial.print("rx from board:");
    //Serial.write((uint8_t *)(buff+pos),len);
    //Serial.println("");
    pos+=len;
    line_cnt++;
    line[line_cnt]=pos;
    len=SerialCon.readBytesUntil('\n',buff+pos,100);
  }
  if(line_cnt>0)Serial.printf("recv size %d byte %d line\r\n",pos,line_cnt);

  

  for(int i=0;i<line_cnt;i++) {
    if(isspace(buff[line[i]])==0) publishSerialData(buff+line[i],line[i+1]-line[i]);
    //Serial.printf("line=%d pos=%d len=%d\r\n",i,line[i],line[i+1]-line[i]);
    //Serial.write((uint8_t *)(buff+line[i]),line[i+1]-line[i]);
    //Serial.println("");
  } 
  

}



void loop__() {
  pubsubClient.loop();
  int line_cnt=0;
  int pos=0;
  int len;
   
  SerialCon.setTimeout(100); 
  len=SerialCon.readBytes(buff+pos,100); 
  while(len>0) {
    
    line[line_cnt++]=pos;
    pos+=len;
    len=SerialCon.readBytes( buff+pos,100); 
    Serial.println(pos);

  }
  if(pos>0) publishSerialData(buff,pos);

}

int loop_cnt=0;
int send_cnt=0;
void loop_() {



    //Net.PowerOn();
    //Net.Connect();
    //Net.HttpGet();
   pubsubClient.loop();
   if (SerialCon.available() > 0) {
     char bfr[501];
     memset(bfr,0, 501);
     SerialCon.readBytesUntil( '\n',bfr,500);
     if(bfr[0]!='\n') {
      publishSerialData(bfr);
     }
   }
   char msg[20] ;
   
   if(loop_cnt++%500000 == 0) {
     send_cnt++;
     sprintf(msg,"test %ld\r\n",send_cnt);
     //publishSerialData(msg);
   }
    //Net.Disconnect();
    //Net.PowerOff();

 }