#include <PubSubClient.h>

#include <WiFi.h>

// Update these with values suitable for your network.
const char* ssid = "vivoV20";
const char* password = "24102002w";

// Config MQTT Server
#define mqtt_server "192.168.43.137"
#define mqtt_port 1883
#define mqtt_user "TEST"
#define mqtt_password "12345"

#define LED_PIN 26

WiFiClient espClient;
PubSubClient client(espClient);

String inputString = "";

int freq = 2000;
int channel = 0;
int resolution = 8;

boolean isNumeric(String str) {
    unsigned int stringLength = str.length();
 
    if (stringLength == 0) {
        return false;
    }
 
    boolean seenDecimal = false;
 
    for(unsigned int i = 0; i < stringLength; ++i) {
        if (isDigit(str.charAt(i))) {
            continue;
        }
 
        if (str.charAt(i) == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }
        return false;
    }
    return true;
}

void setup() {
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(25, channel);

  
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {  
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      return;
    }
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg = "";
  int i=0;
  while (i<length) msg += (char)payload[i++];
  if (msg == "GET") {
    client.publish("led", (digitalRead(LED_PIN) ? "LEDON" : "LEDOFF"));
    Serial.println("Send !");
    return;
  }
  if(msg == "on"){
     digitalWrite(LED_PIN,HIGH);
  }
  if(msg == "off"){
     digitalWrite(LED_PIN,LOW);
  }
  if(isNumeric(msg)){
    int dutyCycle = msg.toInt();
    ledcWrite(channel,dutyCycle);
    Serial.print(dutyCycle);
  }
}
