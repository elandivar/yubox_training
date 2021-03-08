#include <WiFi.h>
#include "OTA.h"
// TODO: esto debe reemplazarse por el mecanismo de cargar credenciales desde preferencias
const char * wifi_ssid = "xxxxxxxx";
const char * wifi_password = "xxxxxxxx";

void setup() {
  WiFi.mode(WIFI_AP_STA);
  //WiFi.softAP(ssidAP, passwordAP):
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) { delay(1000); }

  setupOTA("TemplateSketch");
}

/*******************************************
 ****          LOOP PRINCIPAL           ****
 *******************************************/
 
void loop() {
}
