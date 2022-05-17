#include <WiFi.h>
#include "OTA.h"
// TODO: esto debe reemplazarse por el mecanismo de cargar credenciales desde preferencias
const char * wifi_ssid = "Claro_LANDIVAR";
const char * wifi_password = "0915851372";

void setup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) { delay(1000); }

  setupOTA("CursoYubox");
}

/*******************************************
 ****          LOOP PRINCIPAL           ****
 *******************************************/
 
void loop() {
}
