#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define GAS_SENSOR A0
#define DHT_PIN D5
#define PIEZO_PIN D7
#define DHTTYPE DHT11

const char* ssid = "Galaxy A325BAF";  
const char* password = "tikaygay"; 
const char* host = "sensor-rest-api.herokuapp.com";

DHT dht(DHT_PIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 6*3600);

void setup() 
{ 
  pinMode(GAS_SENSOR,INPUT);
  dht.begin();
  pinMode(PIEZO_PIN, OUTPUT);
  Serial.begin(115200); 
  delay(10);
  Serial.println(); Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP()); 
  timeClient.begin();
} 
  
void loop() 
{  
  timeClient.update();
  time_t rawtime = timeClient.getEpochTime();
  struct tm * timeinfo;
  char buffer[80] = " ";
  timeinfo = localtime(&rawtime);
  strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
  String time = String(buffer);
  time.replace(" ", "%20");
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/report";
  url += "?deviceId=";
  url += "9";
  url += "&temperature=";
  url += (int) dht.readTemperature();
  url += "&damp=";
  url += (int) dht.readHumidity();
  url += "&gas=";
  url += analogRead(GAS_SENSOR);
  url += "&date=";
  url += time;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  String request = "GET " + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "User-Agent: ESP8266\r\n" +
                   "Connection: close\r\n\r\n";
  client.print(request);
  String response = client.readString();
  Serial.println(response);
  client.stop();

  Serial.println();
  Serial.println("closing connection");
  if(analogRead(GAS_SENSOR) > 200) {
    for(int i = 0; i < 40; i++) {
      for (int frequency = 100; frequency < 2000; frequency += 10) {
        tone(PIEZO_PIN, frequency);
        delay(1);
      }
      for (int frequency = 2000; frequency > 100; frequency -= 10) {
        tone(PIEZO_PIN, frequency);
        delay(1);
      }
    }
  } else {
    delay(15200);
  }
  tone(PIEZO_PIN, 0);
}
