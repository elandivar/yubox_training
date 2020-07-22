#include <WiFi.h>

const char* ssid       = "XXXXX";
const char* password   = "XXXXX";

void setup() {
  Serial.begin(115200);
  connectToNetwork();
}

void loop() { 
}

void connectToNetwork() {
  Serial.println("Ingresando a la función connectToNetwork");
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Estableciendo conexión con la red WiFi..");
  }
  Serial.println("Conectado!!!");
}
