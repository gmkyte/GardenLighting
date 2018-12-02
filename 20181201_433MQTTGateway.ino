/*
 433MHz to MQTT Gateway - Under development

 Status: Working

 Description: Receives transmitted 433MHz codes using an RXB6 receiver and publishes the code to MQTT inTopic.
 Takes messages sent to MQTT outTopic and transmits on 433MHz transmitter.

*/

#define D2 4 // I2C Bus SDA (data)
#define D9 3 // RX0 (Serial console)

#include <string.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

RCSwitch mySwitch = RCSwitch();

// Network information

const char* ssid = "COSMOTE-4913A2";
const char* password = "uR4g7j3e4QYqt2SR";
const char* mqtt_server = "192.168.16.18";
const char* myHostName = "433Gateway"; // the name of your fixture, and the base channel to listen to

char msg[16]; //Buffer for the received transmission code

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int value = 0;

void setup_wifi() {

  delay(10);
  
  // Connect to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(myHostName);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  payload[length] = '\0';
  String strPayload = String((char*)payload); //convert character array into String
  Serial.println();
  Serial.print("String Payload: ");Serial.println(strPayload);
  mySwitch.send((strPayload.toInt()), 24);
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin NodeMCU Pin #2, D3
  
  // Transmitter is connected to NodeMCU Pin #2, D4  
  mySwitch.enableTransmit(2);
  // Optional set pulse length.
  mySwitch.setPulseLength(194);
    
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (mySwitch.available()) {
    
    int value = mySwitch.getReceivedValue();

    Serial.print("Transmission code received: "); Serial.println(value);Serial.println();
    
    if (value == 0) {
      Serial.print("Unknown encoding");
      }
      else {
        Serial.print("Publishing: ");
        Serial.println( value );
        itoa(value, msg, 10);
        client.publish("outTopic", msg);
      }
    mySwitch.resetAvailable();
  }
  
}
