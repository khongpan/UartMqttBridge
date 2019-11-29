#include "ArduinoHttpClient.h"
#include "Network.h"  

// Select  modem:
#define TINY_GSM_MODEM_SIM800

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
//#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 38400

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials
const char apn[]  = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <TinyGsmClient.h>

//#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif


Network::Network() {

}

Network::~Network() {

}

//Server details
 const char server[] = "agritronics.nstda.or.th";
 const char resource[] = "/webpost0606/log.php?data1=NODEMCU_002,1000,A,19/02/28,11:36:00,7,10,1,2,3,4";
 const int  port = 80;
 TinyGsmClient client(modem);
 HttpClient http(client, server, port);
 unsigned long last_maintain_t = 0;


Client *Network::GetClient() {
  return  &client;

}

void Network::HttpGet() {

  SerialMon.print(F("Performing HTTP GET request... "));
  int err = http.get(resource);
  if (err != 0) {
    SerialMon.println(F("failed to connect"));
    delay(10000);
    return;
  }

  int status = http.responseStatusCode();
  SerialMon.print(F("Response status code: "));
  SerialMon.println(status);
  if (!status) {
    delay(10000);
    return;
  }

  SerialMon.println(F("Response Headers:"));
  while (http.headerAvailable()) {
    String headerName = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    SerialMon.println("    " + headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0) {
    SerialMon.print(F("Content length is: "));
    SerialMon.println(length);
  }
  if (http.isResponseChunked()) {
    SerialMon.println(F("The response is chunked"));
  }

  String body = http.responseBody();
  SerialMon.println(F("Response:"));
  SerialMon.println(body);

  SerialMon.print(F("Body length is: "));
  SerialMon.println(body.length());

  // Shutdown

  http.stop();
  SerialMon.println(F("Server disconnected"));

}


void Network::Setup() {

  // Set GSM module baud rate
  //TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
  //SerialAT.begin(9600);
  //delay(3000);
}


void Network::Connect() {
  DBG("Connecting to", apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    delay(10000);
    return;
  }

  bool res = modem.isGprsConnected();
  DBG("GPRS status:", res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  DBG("CCID:", ccid);

  String imei = modem.getIMEI();
  DBG("IMEI:", imei);

  String cop = modem.getOperator();
  DBG("Operator:", cop);

  IPAddress local = modem.localIP();
  DBG("Local IP:", local);

  int csq = modem.getSignalQuality();
  DBG("Signal quality:", csq);

  // This is only supported on SIMxxx series
  String gsmLoc = modem.getGsmLocation();
  DBG("GSM location:", gsmLoc);
  String gsmTime = modem.getGSMDateTime(DATE_TIME);
  DBG("GSM Time:", gsmTime);
  String gsmDate = modem.getGSMDateTime(DATE_DATE);
  DBG("GSM Date:", gsmDate);
} 

void Network::Disconnect() {
  modem.gprsDisconnect();
  if (!modem.isGprsConnected()) {
    DBG("GPRS disconnected");
  } else {
    DBG("GPRS disconnect: Failed.");
  }
}

void Network::Maintain() {
 


  if (millis()>(last_maintain_t+5000)) {
    modem.maintain();
    if(modem.isGprsConnected()==false) {
      PowerOff();
      PowerOn();
      Connect();
    }
  }
  last_maintain_t=millis();
}

void Network::PowerOn() {
  
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
  //if (!modem.restart()) {
  if (!modem.init()) {
    DBG("Failed to restart modem, delaying 10s and retrying");
    delay(3000);
    // restart autobaud in case GSM just rebooted
    TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
    delay(10000);
    return;
  }

  String name = modem.getModemName();
  DBG("Modem Name:", name);;

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);

  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
    modem.simUnlock(GSM_PIN);
  }

  DBG("Waiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) {
    DBG("Network connected");
  }


}

void Network::PowerOff() {
  // Try to power-off (modem may decide to restart automatically)
  // To turn off modem completely, please use Reset/Enable pins
  modem.poweroff();
  DBG("Poweroff.");
}


Network Net;