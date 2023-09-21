#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"
#include <ArduinoJson.h> // Library untuk mengelola data JSON

// SSID dan password dari akses point
const char *ssid = "ESP8266-AP";
const char *password = "password";
//IPAddress serverIp(192, 168, 4, 1);
const char* serverIP = "192.168.4.1"; 
const int serverPort = 80;

// Konfigurasi sensor DHT11
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// ESP01 ULTASONIC
// const int TRIGPIN = 2;
// const int ECHOPIN = 0;

// WEMOS ULTASONIC
const int TRIGPIN = 5;
const int ECHOPIN = 4;

WiFiClient client;

bool isConnected = true;

float temp = 0.0;
float hum = 0.0;
float duration;
float distance;

String Message;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TRIGPIN, OUTPUT);
    pinMode(ECHOPIN, INPUT);
    dht.begin();
    Serial.println();

    // Menghubungkan ke akses point 
    Serial.print("Menghubungkan ke ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(2000);
        Serial.println("Menghubungkan ulang...");
    }
    Serial.println("Connected to WiFi");

}

float getDistance() {
  // Replace this with your code to measure distance using the ultrasonic sensor
  // Return the measured distance
  //float distance = 0.0;
  // Your ultrasonic sensor code here
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIGPIN, LOW);
  float duration = pulseIn(ECHOPIN, HIGH);
  float distance = ((duration / 2) * 0.343)/10;
  return distance;
}

void loop()
{
    // Delay selama 1 detik setelah dihidupkan
    delay(1000);
    sendConnectionStatus();
    getTempHum();

    ////////////////////////////// DIRECT SEND TO SERVER START ////////////////////////////////
    // Membaca data ultrasonic
    // digitalWrite(TRIGPIN, LOW);
    // delayMicroseconds(2);
    // digitalWrite(TRIGPIN, HIGH);
    // delayMicroseconds(20);
    // digitalWrite(TRIGPIN, LOW);
    // float duration = pulseIn(ECHOPIN, HIGH);
    // float distance = ((duration / 2) * 0.343)/10;
    

    // // Mengirimkan data ultrasonic ke server
    // WiFiClient client;
    // if (client.connect(serverIp, serverPort))
    // {
    //     Serial.println("Terhubung ke Server");
    //     //digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    //     delay(1200);                     // wait for a second
    //     client.println(distance);
    //     client.println();
    // }
    // else
    // {
    //     Serial.println("Gagal terhubung ke Server");
    //     digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    //     delay(100);
    //     digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
    //     delay(100);                     // wait for a second
    // }

    ////////////////////// DIRECT SEND TO SERVER END ///////////////////////////////


  // Membaca data dari sensor ultrasonik (distance)
  float distance = getDistance();  // Replace this with your code to get distance from ultrasonic sensor

  // Kirim data menggunakan HTTP POST
  sendSensorData(distance, temp, hum);

  // Create an HTTPClient object
  //HTTPClient http;
  
  // Set up the target URL
  //String url = "http://" + String(serverIP) + ":" + String(serverPort) + "/distance/";
  
  // Start the POST request
  //http.begin(url);
  //http.addHeader("Content-Type", "text/plain");
  
  // Send the data as the POST body
  //int httpCode = http.POST(String(distance));
  
  // if (httpCode > 0) {
  //   String response = http.getString();
  //   Serial.println("Server response: " + response);
  //   Serial.println("Terhubung ke Server");

  //   //digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  //   delay(1200);                     // wait for a second
  //   client.println(distance);
  //   client.println();
  // } else {
  //   Serial.println("HTTP POST failed");
  //   Serial.println("Gagal terhubung ke Server");
  //   digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  //   delay(100);
  //   digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  //   delay(100);                     // wait for a second
  // }


  // Close the connection
  //http.end();

  delay(1200);  // Send data every seconds
}


void getTempHum() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  
  Message += " Temperature: ";
  Message += temp;
  Message += " *C Humidity: ";
  Message += hum;
  Message += " %RH ";

  Serial.println(Message);

  //Serial.println("Distance received from client: " + String(distance));
}

// CONNECTION TO SERVER
void sendConnectionStatus() {
    HTTPClient http;

    String url = "http://" + String(serverIP) + ":" + String(serverPort) + "/updateConnectionStatus/";
    http.begin(url);
    http.addHeader("Content-Type", "text/plain");

    int httpCode = http.POST(String(isConnected));
    Serial.println(isConnected);

    if (httpCode > 0) {
        Serial.println("Connection status sent to server");
    } else {
        Serial.println("Failed to send connection status");
    }

    http.end();
}


void sendSensorData(float distance, float temp, float hum) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://" + String(serverIP) + ":" + String(serverPort) + "/data";

    // Buat objek JSON
    StaticJsonDocument<200> jsonDoc; // Ukuran buffer JSON
    jsonDoc["distance"] = distance;
    jsonDoc["temperature"] = temp;
    jsonDoc["humidity"] = hum;

    // Mengonversi objek JSON menjadi string JSON
    String jsonStr;
    serializeJson(jsonDoc, jsonStr);

    // Kirim data menggunakan permintaan HTTP POST
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonStr);

    Serial.print("Kode Status HTTP: ");
    Serial.println(httpCode);
    String response = http.getString();
    Serial.println("Respons dari Server: " + response);

    http.end();
  }
}
