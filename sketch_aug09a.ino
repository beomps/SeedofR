#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

/* Humid & temp Sensor */
#define DHTPIN D4 //D4 Pin
#define DHTTYPE DHT11

/* UV Sensor */
#define sensorUVPin A0 //Analog Pin
int dataSensorUV = 0;
int indexUV = 0;

/* Light Sensor */
const int lightPin = D5;  //D2 Pin
char* light_status;

/* Rain Sensor */
const int rainPin = D1;  //D1 Pin
char* rain_status;

/* Water Flow Sensor */
const byte flowsensor = D2; // Sensor Input
const byte flowsensor2 = D7; // Sensor Input
void ICACHE_RAM_ATTR ISRoutine();
void ICACHE_RAM_ATTR ISRoutine2();
float calibrationFactor = 4.5;
float calibrationFactor2 = 4.5;
volatile byte pulseCount;  
volatile byte pulseCount2;  
float flowRate;
float flowRate2;
unsigned int flowMilliLitres;
unsigned int flowMilliLitres2;
unsigned long totalMilliLitres;
unsigned long totalMilliLitres2;
unsigned long oldTime;
unsigned long oldTime2;

/* Wifi Settings */
const char* ssid = "kb";
const char* password = "11301234ab";
const char* mqtt_server = "192.168.29.134";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
String packet;
float Humi,Temp;

DHT dht(DHTPIN, DHTTYPE);



void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(lightPin, INPUT);
  pinMode(rainPin, INPUT);
  //flow sensor
  pinMode(flowsensor, INPUT_PULLUP);
  pinMode(flowsensor2, INPUT_PULLUP);
  pulseCount        = 0;
  pulseCount2        = 0;
  flowRate          = 0.0;
  flowRate2          = 0.0;
  flowMilliLitres   = 0;
  flowMilliLitres2   = 0;
  totalMilliLitres  = 0;
  totalMilliLitres2  = 0;
  oldTime           = 0;
  oldTime2           = 0;
  attachInterrupt(digitalPinToInterrupt(flowsensor),ISRoutine,FALLING);
  attachInterrupt(digitalPinToInterrupt(flowsensor2),ISRoutine2,FALLING);
  //end
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
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
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
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


float getHumi() { //DHT11 습도를 받아오는 함수
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  Serial.print("Humi: ");
  Serial.print(h);
  Serial.print(" %\t");
  return(h);
}

float getTemp() {//DHT11 온도를 받아오는 함수
  
  float t = dht.readTemperature();

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.println(" *C ");

  return(t);
}

/* Read UV Sensor in mV and call UV index calculation */
void readSensorUV()
{                    
  byte numOfReadings = 5;
  dataSensorUV = 0;
  for (int i=0; i< numOfReadings; i++) 
  {
    dataSensorUV += analogRead(sensorUVPin);
    delay (200);
  }
  dataSensorUV /= numOfReadings;
  dataSensorUV = (dataSensorUV * (3.3 / 1023.0))*1000;
  Serial.println(dataSensorUV);
  indexCalculate();
}

/*  UV Index calculation */
void indexCalculate()
{
    if (dataSensorUV < 227) indexUV = 0;
    else if (227 <= dataSensorUV && dataSensorUV < 318) indexUV = 1;
    else if (318 <= dataSensorUV && dataSensorUV < 408) indexUV = 2;
    else if (408 <= dataSensorUV && dataSensorUV < 503) indexUV = 3;
    else if (503 <= dataSensorUV && dataSensorUV < 606) indexUV = 4;    
    else if (606 <= dataSensorUV && dataSensorUV < 696) indexUV = 5;
    else if (696 <= dataSensorUV && dataSensorUV < 795) indexUV = 6;
    else if (795 <= dataSensorUV && dataSensorUV < 881) indexUV = 7; 
    else if (881 <= dataSensorUV && dataSensorUV < 976) indexUV = 8;
    else if (976 <= dataSensorUV && dataSensorUV < 1079) indexUV = 9;  
    else if (1079 <= dataSensorUV && dataSensorUV < 1170) indexUV = 10;
    else indexUV = 11;       
}

void ISRoutine () {
     pulseCount++;
}
void ISRoutine2 () {
     pulseCount2++;
}

void mqtt_publish(float Humi, float Temp, int dataSensorUV, int indexUV, char* light_status, char* rain_status){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;


    Serial.println("Humi : " + String(Humi) + "% " + "Temp : " + String(Temp) + "*C" + " UV :" + String(dataSensorUV) + " UVI:" + String(indexUV) +" Light:" + String(light_status)+" Rain :" + String(rain_status));
    packet = "Humi : " + String(Humi) + "% " + "Temp : " + String(Temp) + "*C" + " UV :" + String(dataSensorUV) + " UVI:" + String(indexUV) +" Light:" + String(light_status)+" Rain :" + String(rain_status);
    packet.toCharArray(msg, 50); 
    //mqtt publishing이 char형으로만 보낼 수 있기때문에 toCharArray로 변환한다.
    //Serial.print("Publish message: ");
    //Serial.println(msg);
    client.publish("Sensor/Humi_Temp", msg);
  }
  delay(5000); //50초 단위로 Publishing (조정가능)
}

void loop() {
  Humi = getHumi(); //습도를 받아서 변수에 입력
  Temp = getTemp(); //온도를 받아서 변수에 입력

  readSensorUV(); //Get UV value

  if (digitalRead(lightPin)==0){
    light_status = "On";
  }else { 
    light_status = "Off"; 
  }

  if (digitalRead(rainPin)==0){
    rain_status = "Rainy";
  }else{
    rain_status = "none";
  }
  
  //water flow sensor
  if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    detachInterrupt(digitalPinToInterrupt(flowsensor));     
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;  
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
    Serial.print(totalMilliLitres/1000);
    Serial.print("L");
 
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(digitalPinToInterrupt(flowsensor),ISRoutine,FALLING);
  }
  
  if((millis() - oldTime2) > 1000)    // Only process counters once per second
  { 
    detachInterrupt(digitalPinToInterrupt(flowsensor2));     
    flowRate2 = ((1000.0 / (millis() - oldTime2)) * pulseCount2) / calibrationFactor;
    oldTime2 = millis();
    flowMilliLitres2 = (flowRate2 / 60) * 1000;
    totalMilliLitres2 += flowMilliLitres2;  
    unsigned int frac2;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow2 rate: ");
    Serial.print(int(flowRate2));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid2 Quantity: ");        
    Serial.print(totalMilliLitres2);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
    Serial.print(totalMilliLitres2/1000);
    Serial.print("L");
 
    // Reset the pulse counter so we can start incrementing again
    pulseCount2 = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(digitalPinToInterrupt(flowsensor2),ISRoutine2,FALLING);
  }
  
  Serial.println("Humi : " + String(Humi) + "% " + "Temp : " + String(Temp) + "*C" + " UV :" + String(dataSensorUV) + " UVI:" + String(indexUV) +" Light:" + String(light_status)+" Rain :" + String(rain_status));

  //mqtt_publish(Humi, Temp, dataSensorUV, indexUV, light_status, rain_status);// 온습도의 값을 함수에 넣어서 해당 값을 통신을 통해서 전송한다.
};
