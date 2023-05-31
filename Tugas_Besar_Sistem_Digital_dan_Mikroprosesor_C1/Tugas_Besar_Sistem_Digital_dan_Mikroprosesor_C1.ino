/*EL2142 Sistem Digital dan Mikroprosesor
 *Modul             : Tugas Besar
 *Hari dan Tanggal  : 9 Desember 2022
 *Nama (NIM) 1      : Kayyisa Zahratulfirdaus (18320011)
 *Nama (NIM) 2      : Namira Husaimah (18319039)
 *Nama File         : 
 *Deskripsi         : Program alat bantu jalan untuk tunanetra
 *                    
 *                    
 */

#include <Arduino.h>
#include "ESP32_MailClient.h"
#include <WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

const char *SSID = "Nama Wifi";
const char *PWD = "Password Wifi";
#define GMAIL_SMTP_SEVER "smtp.gmail.com"
#define GMAIL_SMTP_USERNAME "Alamat Email"
#define GMAIL_SMTP_PASSWORD "yyvkvlucckfrohjj"
#define GMAIL_SMTP_PORT 465 

unsigned long previousMillis = 0;
unsigned long interval = 30000;

//Init Button
const int buttonPin = 15;
int buttonState = 0;

//Init GPS
static const int RXPin = 23, TXPin = 21;
static const uint32_t GPSBaud = 9600;

//Init Sensor
const unsigned int TRIG_PIN=33; //arduino 13
const unsigned int ECHO_PIN=13; //arduino 12
const unsigned int BZZ_PIN=18; // arduino 3
const unsigned int BAT_PIN=34; // arduino 3

float battery;
float bat_percentage;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
 
// EMail Data
SMTPData data;
void connectToWiFi() {
  Serial.print("Connectiog to ");
 
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected - ");
  //Serial.println(WiFi.localIP);
}

String sendEmail(char *subject, char *sender, char *body, char *recipient, boolean htmlFormat) {
  data.setLogin(GMAIL_SMTP_SEVER, GMAIL_SMTP_PORT, GMAIL_SMTP_USERNAME, GMAIL_SMTP_PASSWORD);
  data.setSender(sender, GMAIL_SMTP_USERNAME);
  data.setSubject(subject);
  data.setMessage(body, htmlFormat);
  data.addRecipient(recipient);
  if (!MailClient.sendMail(data)) 
    return MailClient.smtpErrorReason();
  
  return "";
}

void setup() {
  Serial.begin(9600);
  
  // Connect to WiFi
  connectToWiFi();
  
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  // GPS
  ss.begin(GPSBaud);

  //Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BZZ_PIN, OUTPUT);
  pinMode(BAT_PIN, INPUT);

}

void loop() {
  //Init
  char body[2048]; 
  float latitude = 0;
  float longitude = 0;
  
  //Rechecking Connection
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }

  //GPS
   while (ss.available() > 0){
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      latitude = gps.location.lat();
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
      longitude = gps.location.lng();
    }
  }


  //battery = 1.13;
  int voltage_read = analogRead(BAT_PIN);
  battery = (voltage_read * 3.3) / 4095;
  bat_percentage = (battery * 100)/ 1.8;


    Serial.print("Baterai (V): ");
    Serial.println(battery);
    Serial.print("Baterai (%): ");
    Serial.println(int(bat_percentage));

  
  //Low Battery Notification
  if (battery <= 1.13){
    //Define the email body
    sprintf(body, "<h2>Emergency</h2><br/><p><b>Peringatan! Baterai Habis. Lokasi terakhir di: </b></p><br/><p><b>www.google.com/maps/place/%.14f,%.14f</font></p>", latitude, longitude);
    String result = sendEmail("Alat Bantu Jalan Tuna Netra", "ESP32", body, "adeliazeta@gmail.com", true);

    Serial.println("Peringatan, baterai lemah!");
    digitalWrite(BZZ_PIN, HIGH);
    delay(500);
    digitalWrite(BZZ_PIN, LOW);
    delay(500);
    digitalWrite(BZZ_PIN, HIGH);
    delay(500);
    digitalWrite(BZZ_PIN, LOW);
    delay(500);
    digitalWrite(BZZ_PIN, HIGH);
    delay(500);
    digitalWrite(BZZ_PIN, LOW);
  }

  

  //Distance Detection
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  

  const unsigned long duration= pulseIn(ECHO_PIN, HIGH);
  int distance= duration/29/2;
  if(duration==0){
   Serial.println("Warning: no pulse from sensor");
   } 
  else{
      Serial.print("distance to nearest object:");
      Serial.print(distance);
      Serial.println(" cm");
      Serial.println();
      Serial.println();
  
      //Object Detection
      if (distance <= 50){
        digitalWrite(BZZ_PIN, HIGH);
      }
      else{
        digitalWrite(BZZ_PIN, LOW);
      }
    }
   delay(100);

  //Define the email body
  sprintf(body, "<h2>Emergency</h2><br/><p><b>Email ini dikirim ketika emergency button pada alat bantu jalan tuna netra dipencet. Tolong jemput saya dengan lokasi di: </b></p><br/><p><b>www.google.com/maps/place/%.14f,%.14f</font></p>", latitude, longitude);

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  
  if (buttonState == HIGH) {
    // Sending the email
    String result = sendEmail("Alat Bantu Jalan Tuna Netra", "ESP32", body, "adeliazeta@gmail.com", true);
    digitalWrite(BZZ_PIN, HIGH);
    delay(500);
    digitalWrite(BZZ_PIN, LOW);
    delay(500);
    digitalWrite(BZZ_PIN, HIGH);
    delay(500);
    digitalWrite(BZZ_PIN, LOW);
  }
  
}
