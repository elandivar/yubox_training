#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi settings
const char* ssid = "CLARO-ELCH";
const char* password = "cursoyubox";
HTTPClient http;


void setup() {
  Serial.begin(115200);
  //Wi-Fi connection
  Serial.print("Connecting to the................");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);
  //WiFi.begin(ssid);
    
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("WiFi connected.");
  Serial.print("My IP address: ");  
  Serial.println(WiFi.localIP());   

  bool status;
    
  delay(100); // let sensor boot up

}

void loop() {
  sendData();
  delay(10000);
}

void sendData() {
    String url = "http://yubox.com/training_chat.php?str=COMPU1";
    http.begin(url);
    int httpCode = http.GET();
    http.writeToStream(&Serial);
    http.end();
    Serial.println("");  
}
