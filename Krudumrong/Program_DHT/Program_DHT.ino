#include <PubSubClient.h>
#include <WiFi.h>
#include "DHT.h"

const char* ssid = "vivoV20";
const char* password = "24102002w";

// Config MQTT Server
#define mqtt_server "192.168.43.137"
#define mqtt_port 1883
#define mqtt_user "TEST"
#define mqtt_password "12345"

#define LED_PIN 26

#define DHTPIN 4
#define DHTTYPE DHT22 

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

int freq = 2000;
int channel = 0;
int resolution = 8;


unsigned long time1 = millis();

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

void runmqtt(){

  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("led");
      client.subscribe("run");

      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      return;
    }
  }
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg = "";
  int i=0;
  while (i<length) msg += (char)payload[i++];

  Serial.println(msg);
//  client.publish("temp","asdfa");
  if(msg == "on"){
     digitalWrite(LED_PIN,HIGH);   
  }
  if(msg == "off"){
     digitalWrite(LED_PIN,LOW);   
  } 
  if(isNumeric(msg)){
    ledcWrite(channel,msg.toInt());
  } 

}


void readdht(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  
  String temp = String(t);
  String faren = String(f);
  String humanity = String(h);
  client.publish("temp",(char*) temp.c_str());
  client.publish("faren",(char*) faren.c_str());
  client.publish("humanity",(char*) humanity.c_str());
}


void setup() {
  // put your setup code here, to run once:
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
  Serial.println(F("DHT22 test!"));

  dht.begin();
    
}



void loop() {
  // put your main code here, to run repeatedly:


  runmqtt();  
  client.loop();
  if (millis() - time1 >  2000){
    readdht();
    time1 += 1000; 
    }

}
