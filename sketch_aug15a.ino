/*
 Basic MQTT example
 
 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
 
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Base64.h>

const char* CODE_CONNECTED = "CONN";
const char* CODE_NORMAL = "NORM";
const char* CODE_QUEIMADA = "QUEI";
const char* CODE_ACESA = "ACES";
 
// Configurações do servidor MQTT
const char* mqtt_server = "200.143.224.99";
const int mqtt_port = 1883;
const char* broker_topic = "broker_antonio";
 
// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 67, 120);
IPAddress server(200, 143, 224, 99);

// Configurações dos sensores
int ambienceThreshold = 100;
int lampThreshold = 100;

// Configurações do dispositivo
EthernetClient ethClient;
PubSubClient client(ethClient);
long baud = 57600;

String encrypt(char* inputString) {
  int inputStringLength = strlen(inputString);

  int encodedLength = Base64.encodedLength(inputStringLength);
  char encodedString[encodedLength + 1];
  Base64.encode(encodedString, inputString, inputStringLength);

  return encodedString;
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Estabelecendo conexão MQTT... ");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("conectado");
      // Once connected, publish an announcement...
      client.publish(broker_topic, encrypt(CODE_CONNECTED).c_str());
      // ... and resubscribe
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int prevState = 0;

void trackState(int state) {
  if (prevState != state) {
    prevState = state;
    char* message = "";
    switch (state) {
      case 1:
        digitalWrite(LED_BUILTIN, HIGH);
        message = CODE_QUEIMADA;
        break;
      case 2:
        digitalWrite(LED_BUILTIN, HIGH);
        message = CODE_ACESA;
        break;
      default:
        digitalWrite(LED_BUILTIN, LOW);
        message = CODE_NORMAL;
        break;
    }
    client.publish(broker_topic, encrypt(message).c_str());
  }
}

void setup()
{
  Serial.begin(baud);
  Serial.println("Dispositivo operante. Iniciando conexão por DHCP...");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
 
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
 
  Ethernet.begin(mac);
  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{
   if (!client.connected()) {
     reconnect();
   }
   
  if (analogRead(A0) > lampThreshold && analogRead(A1) > ambienceThreshold) {
    delay(1000);
    if (analogRead(A0) > lampThreshold && analogRead(A1) > ambienceThreshold) {
      trackState(1);
    }
  } else if (analogRead(A0) < lampThreshold && analogRead(A1) < ambienceThreshold) {
    delay(1000);
    if (analogRead(A0) < lampThreshold && analogRead(A1) < ambienceThreshold) {
      trackState(2);
    }
  } else {
      trackState(0);
  }
  delay(500);
  client.loop();
}