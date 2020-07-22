#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "ClosedCube_SHT31D.h"

// Wi-Fi settings
const char* ssid = "XXXXXXX";
const char* password = "XXXXXXX";

ClosedCube_SHT31D sht3xd;
HTTPClient http;

void setup()
{
	Wire.begin(32,25);

	Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Conectado al WiFi con IP: ");
  Serial.println(WiFi.localIP());
  
	sht3xd.begin(0x44); // I2C address: 0x44 or 0x45

	Serial.print("Sensor Serial #");
	Serial.println(sht3xd.readSerialNumber());

	if (sht3xd.periodicStart(SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_10HZ) != SHT3XD_NO_ERROR)
		Serial.println("[ERROR] No se pudo inicializar el sensor");

}

void loop()
{
	printResult("Periodic Mode", sht3xd.periodicFetchData());
	delay(20000);
}

void printResult(String text, SHT31D result) {
	if (result.error == SHT3XD_NO_ERROR) {

    String yuboxbroker_url = "http://broker.yubox.com:5555/device/d3c85b8315a159d6c10d650feab915b9/params/" + String(result.t);
    Serial.println(yuboxbroker_url.c_str());
    http.begin(yuboxbroker_url.c_str());

    int httpResponseCode = http.GET();

    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources

    http.end();
    
	} else {
		Serial.print(text);
		Serial.print(": [ERROR] Code #");
		Serial.println(result.error);
	}
}
