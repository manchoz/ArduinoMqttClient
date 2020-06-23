/*
  ArduinoMqttClient - GSM JSON Sender

  This example connects to a MQTT broker and publishes a JSON
  message to a topic once a second.

  The circuit:
  - Arduino MKR 1400
  - Antenna
  - SIM card with a data plan

  This example code is in the public domain.
*/

#include <ArduinoMqttClient.h>
#include <MKRGSM.h>
#include <Arduino_JSON.h>

#include "arduino_secrets.h" 
// Please enter your sensitive json in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[]     = SECRET_PINNUMBER;
// APN json
const char GPRS_APN[]      = SECRET_GPRS_APN;
const char GPRS_LOGIN[]    = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

// To connect with SSL/TLS:
// 1) Change GSMClient to GSMSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the GSM module.

GSMModem modem;
GSM gsmAccess(true);
GPRS gprs;
GSMClient client;
MqttClient mqttClient(client);

const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "arduino/json";

const long interval = 1000;
unsigned long previousMillis = 0;

int count = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // connection state
  bool connected = false;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected) {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("You're connected to the network");
  Serial.println();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  mqttClient.setId(modem.getICCID());

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    String message = createJson();
    Serial.print("Sending message to topic: ");
    Serial.println(message);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(message);
    mqttClient.endMessage();

    Serial.println();

    count++;
  }
}

String createJson()
{
  JSONVar json;

  json["id"] = modem.getICCID();
  json["uptime"] = int { millis() / 1000 };
  json["count"] = count;
  json["compile"]["date"] = __DATE__;
  json["compile"]["time"] = __TIME__;

  return JSON.stringify(json);
}