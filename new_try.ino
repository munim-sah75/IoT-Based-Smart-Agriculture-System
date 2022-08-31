/**********************************************************************************
 *  TITLE: MQ2 Gas Leakage & Rain Detector with Blynk IoT using NodeMCU ESP8266
 *  Click on the following links to learn more. 
 *  YouTube Video: 
 *  Related Blog : https://iotcircuithub.com/mq2-gas-sensor-rain-detection-blynk-notification
 *  by Tech StudyCell
 *  Preferences--> Aditional boards Manager URLs : 
 *  https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *  
 *  Download Board ESP8266 NodeMCU : https://github.com/esp8266/Arduino
 *
 *  Download the libraries 
 *  Blynk 1.0.1 Library:  https://github.com/blynkkk/blynk-library
 **********************************************************************************/

/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID "TMPLeXlMitUT"
#define BLYNK_DEVICE_NAME "Rain and Gas"
#define BLYNK_AUTH_TOKEN "lEXq6LLFAD2ARb_SB2K8uPRYXh2aCaho"

// Your WiFi Credentials.
// Set password to "" for open networks.
char ssid[] = "Stop Pinching";
char pass[] = "roomkey8";

// define the GPIO connected with Sensors & LEDs
#define MQ2_SENSOR    A0 //A0
#define RAIN_SENSOR   5  //D1
#define GREEN_LED     14 //D5
#define RED_LED       13 //D7
#define WIFI_LED      16 //D0
#define TRIGGERPIN D3
#define ECHOPIN    D2
#define I2C_SCL D4
#define I2C_SDA D6


#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_Sensor.h>
Adafruit_BMP085 bmp;

float dst,bt,bp,ba;
char dstmp[20],btmp[20],bprs[20],balt[20];
bool bmp085_present=true;

int MQ2_SENSOR_Value = 0;
int RAIN_SENSOR_Value = 0;
bool isconnected = false;
char auth[] = BLYNK_AUTH_TOKEN;

#define VPIN_BUTTON_1    V1 
#define VPIN_BUTTON_2    V2

long tankDepth = 11;

BlynkTimer timer;

void checkBlynkStatus() { // called every 2 seconds by SimpleTimer
  getSensorData();
  isconnected = Blynk.connected();
  if (isconnected == true) {
    digitalWrite(WIFI_LED, LOW);
    sendSensorData();
    //Serial.println("Blynk Connected");
  }
  else{
    digitalWrite(WIFI_LED, HIGH);
    Serial.println("Blynk Not Connected");
  }
}

void getSensorData()
{
  MQ2_SENSOR_Value = map(analogRead(MQ2_SENSOR), 0, 1024, 0, 100);
  RAIN_SENSOR_Value = digitalRead(RAIN_SENSOR);
  if (MQ2_SENSOR_Value > 50 ){
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }
  else if (RAIN_SENSOR_Value == 0 ){
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }
  else{
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  }

}

void sendSensorData()
{  
  Blynk.virtualWrite(VPIN_BUTTON_1, MQ2_SENSOR_Value);
  if (MQ2_SENSOR_Value > 50 )
  {
    Blynk.logEvent("gas", "Gas Detected!");
  }
  if (RAIN_SENSOR_Value == 0 )
  {
    Blynk.logEvent("rain", "Water Detected!");
    Blynk.virtualWrite(VPIN_BUTTON_2, "Water Detected!");
  }
  else if (RAIN_SENSOR_Value == 1 )
  {
    Blynk.virtualWrite(VPIN_BUTTON_2, "No Water Detected.");
  } 
    if (!bmp.begin()) 
  {
        Serial.println("Could not find a valid BMP085 sensor, check wiring!");
        while (1) {}
        }

//float h = dht.readHumidity();
//float t = dht.readTemperature();
  

  //if (isnan(h) || isnan(t))
  //{
  //Serial.println("Failed to read from DHT sensor!");
  //return;
  //}

//double gamma = log(h/100) + ((17.62*t) / (243.5+t));
//double dp = 243.5*gamma / (17.62-gamma);

  float bp =  bmp.readPressure()/100;
  float ba =  bmp.readAltitude();
  float bt =  bmp.readTemperature();
  float dst = bmp.readSealevelPressure()/100;

//Blynk.virtualWrite(V5 , h);
//Blynk.virtualWrite(V6 , t);
Blynk.virtualWrite(V5, bp);
Blynk.virtualWrite(V4, ba);
Blynk.virtualWrite(V3, bt);
Blynk.virtualWrite(V6, dst);
//Blynk.virtualWrite(V14, dp);
Serial.print("Pressure: ");
Serial.print(bp);
Serial.print("Temperature: ");
Serial.print(bt);
Serial.print("Altitude: ");
Serial.print(ba);
Serial.print("Sea: ");
Serial.print(dst);
}

void setup()
{
  Serial.begin(9600);
 
  pinMode(TRIGGERPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
  pinMode(MQ2_SENSOR, INPUT);
  pinMode(RAIN_SENSOR, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(WIFI_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(WIFI_LED, HIGH);

  WiFi.begin(ssid, pass);
  Wire.begin(I2C_SDA, I2C_SCL);
  timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  Blynk.config(auth);
  delay(1000);
}

void loop()
{
  long duration, distance;
  digitalWrite(TRIGGERPIN, LOW);  
  delayMicroseconds(2); 

  digitalWrite(TRIGGERPIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGERPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);
  distance = (duration/2) / 29.1;
  Serial.print(distance);
  Serial.println("Cm");
  double level = tankDepth - distance;
  if (level > 0) {
    long percentage = ((level / tankDepth)) * 100;
    Blynk.virtualWrite(V0, percentage);
  }
  else Blynk.virtualWrite(V0, 0);
  
  Blynk.run();
  timer.run();
  delay(500);
}
