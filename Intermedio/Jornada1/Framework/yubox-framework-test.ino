#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define ARDUINOJSON_USE_LONG_LONG 1

#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "YuboxWiFiClass.h"
#include "YuboxNTPConfigClass.h"
#include "YuboxOTAClass.h"
#include "YuboxMQTTConfClass.h"

AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *);

// El modelo viejo de YUBOX tiene este sensor integrado en el board
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 sensor_bmp280;

AsyncEventSource eventosLector("/yubox-api/lectura/events");

void setup()
{
  // La siguiente demora es sólo para comodidad de desarrollo para enchufar el USB
  // y verlo en gtkterm. No es en lo absoluto necesaria como algoritmo requerido.
  delay(3000);
  Serial.begin(115200);

  Serial.println("DEBUG: inicializando SPIFFS...");
  if (!SPIFFS.begin(true)) {
    Serial.println("ERR: ha ocurrido un error al montar SPIFFS");
    return;
  }

  // Limpiar archivos que queden de actualización fallida
  YuboxOTA.cleanupFailedUpdateFiles();

  // Activar y agregar todas las rutas que requieren autenticación
  YuboxWebAuth.setEnabled(true);	// <-- activar explícitamente la autenticación
  AsyncWebHandler &h = server.serveStatic("/", SPIFFS, "/");
  YuboxWebAuth.addManagedHandler(&h);
  YuboxWebAuth.addManagedHandler(&eventosLector);
  server.addHandler(&eventosLector);

  YuboxWiFi.begin(server);
  YuboxWebAuth.begin(server);
  YuboxNTPConf.begin(server);
  YuboxOTA.begin(server);
  YuboxMQTTConf.begin(server);
  server.onNotFound(notFound);
  server.begin();

  if (!sensor_bmp280.begin(BMP280_ADDRESS_ALT)) {
    Serial.println("ERR: no puede inicializarse el sensor BMP280!");
  }
}

void loop()
{
  YuboxNTPConf.update();
  if (!YuboxNTPConf.isNTPValid()) {
    if (WiFi.isConnected()) Serial.println("ERR: fallo al obtener hora de red");
  } else {
    DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(3));
    json_doc["ts"] = 1000ULL * YuboxNTPConf.getUTCTime();
    json_doc["temperature"] = sensor_bmp280.readTemperature();
    json_doc["pressure"] = sensor_bmp280.readPressure();

    String json_output;
    serializeJson(json_doc, json_output);

    if (WiFi.isConnected() && eventosLector.count() > 0) {
      eventosLector.send(json_output.c_str());
    }
  }

  delay(3000);
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"msg\":\"El recurso indicado no existe o no ha sido implementado\"}");
}
