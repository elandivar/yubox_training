#include <WiFi.h>
#include <AsyncMqttClient.h>

// TODO: esto debe reemplazarse por el mecanismo de cargar credenciales desde preferencias
const char * wifi_ssid = "xxxxxxxx";
const char * wifi_password = "xxxxxxxx";

#define MQTT_HOST "host-name"
#define MQTT_PORT 1883
#define MQTT_USERNAME "xxxxxxx"
#define MQTT_PASSWORD "xxxxxxx"
#define MQTT_CLIENTID "YUBOX-TEST1" // Este identificador tiene que ser único

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

void connectToWifi()
{
  Serial.println("Iniciando conexión a WiFi...");
  String mDNS_Hostname = generarHostnameMAC();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(mDNS_Hostname.c_str());
  WiFi.begin(wifi_ssid, wifi_password);
}


void connectToMqtt()
{
  Serial.printf("Iniciando conexión a MQTT en %s:%u...\r\n", MQTT_HOST, MQTT_PORT);
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\r\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("Conectado al WiFi.");
        Serial.println("Dirección IP: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("Se perdió conexión WiFi.");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}


void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  uint16_t packetIdSub = mqttClient.subscribe("netpify/muestrapotencia/+", 1);
  Serial.print("Subscribing at QoS 1, packetId: ");
  Serial.println(packetIdSub);

  uint16_t packetIdPub1 = mqttClient.publish(
    "yubox/testing/" MQTT_CLIENTID, // Tópico sobre el cual publicar
    1,                              // QoS (0,1,2)
    false,                          // Poner a true para retener el mensaje luego de publicado. Nuevos clientes verán el mensaje luego de publicado.
    "{Temp: 34}"                  // Payload, cadena de C delimitada por \0 si no se especifica parámetro siguiente de longitud
    // longitud_de_payload
    );
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  // La siguiente demora es sólo para comodidad de desarrollo para enchufar el USB
  // y verlo en gtkterm. No es en lo absoluto necesaria como algoritmo requerido.
  delay(3000);
  Serial.begin(115200);

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
  mqttClient.setClientId(MQTT_CLIENTID);

  connectToWifi();
}

String generarHostnameMAC(void)
{
  byte maccopy[6];
  memset(maccopy, 0, sizeof(maccopy));
  WiFi.macAddress(maccopy);

  String hostname = "YUBOX-";
  for (auto i = 0; i < sizeof(maccopy); i++) {
    String octet = String(maccopy[i], 16);
    octet.toUpperCase();
    hostname += octet;
  }
  return hostname;
}

void loop() {
  // put your main code here, to run repeatedly:

}
