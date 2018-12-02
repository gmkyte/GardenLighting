/*

Graham's Hue Light Controller, MQTT, OpenHab2

*/
#include <string.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

#include <FastLED.h>
#define NUM_LEDS 12
#define DATA_PIN 2 // This is Pin (D4) on the NodeMCU Dev Board (GPIO2 on the ESP8266 bareboard)

int hue = 0;
int sat = 0;
int val = 0;

char* myHue;
char* mySat;
char* myVal;

String myHueS;
String mySatS;
String myValS;

const char* ssid = "ssid";
const char* password = "password";
const char* mqtt_server = "192.168.16.18";
const char* myHostName = "loungelight"; // the name of your fixture, and the base channel to listen to

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const char* outTopic = "Report";
const char* inTopic = myHostName;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.hostname(myHostName);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Waiting for message");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  payload[length] = '\0';
  String strPayload = String((char*)payload);
  Serial.println();
  Serial.print("String Payload: ");Serial.println(strPayload);
  Serial.println();

  char *myMessage = (char *) payload;
  myMessage[length] = '\0';

  if (strPayload == "ON"){
      Serial.println("Full brightness ON");
      FastLED.showColor(CHSV(128,0,255));
      }
      else if (strPayload == "OFF"){
       Serial.println("Fully powered OFF");
       FastLED.showColor(CHSV(128,128,0));
      }
      else {
        // Returns first token 
        char* token = strtok(myMessage, ",");
        myHue = token;
        charToString(myHue,myHueS);
        hue = (myHueS.toInt());
        token = strtok(NULL, ",");
        mySat = token;
        charToString(mySat,mySatS);
        sat = (mySatS.toInt());
        token = strtok(NULL, ",");
        myVal = token;
        charToString(myVal,myValS);
        val = (myValS.toInt());

        hue = map(hue,0,360,0,255);
        sat = map(sat,0,100,0,255);
        val = map(val,0,100,0,255);
        
        Serial.print("Hue: ");Serial.println(hue);
        Serial.print("Sat: ");Serial.println(sat);
        Serial.print("Val: ");Serial.println(val);

  //      hue = map(hue,0,360,0,255);

        FastLED.showColor(CHSV(hue, sat, val));
        }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "Reporting....");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for(int i = 0; i<5000; i++){
        delay(1);
      }
    }
  }
}


void setup() {
  
  CRGB leds[NUM_LEDS];
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);//Use RGB for single CoolComponents LED
  Serial.begin(115200);
  setup_wifi();                   // Connect to wifi 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void charToString(char S[], String &D)
{
  String rc(S);
 D = rc;
 
}
