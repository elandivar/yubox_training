#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HDC2080.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define ADDR 0x41
HDC2080 sensor(ADDR);

#define SCREEN_WIDTH 128 // ancho de la pantalla OLED
#define SCREEN_HEIGHT 32 // altura de la pantalla OLED

// Declara una instancia del objeto SSD1306 llamada display.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Define las credenciales de Wi-Fi
const char* ssid     = "SSID";
const char* password = "PASSWORD";

// Define las credenciales del servidor MQTT
const char* mqtt_server = "HOST";
const char* mqtt_user = "USER";
const char* mqtt_password = "PASS";

// Define los topics MQTT
const char* temp_topic = "sensor/temperature";
const char* hum_topic = "sensor/humidity";

// Define el intervalo de envío de datos en milisegundos (15 segundos)
const long publish_interval = 15000;

// Inicializa el cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Contador para los puntos de intento de conexión
int connectionAttempt = 0;

// Variables para el control de tiempo de publicación de datos
long lastPublishMillis = 0;
long lastSuccessfulPublishMillis = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial);

  // Initialize I2C communication
  sensor.begin();

  // Begin with a device reset
  sensor.reset();

  // Set up the comfort zone
  sensor.setHighTemp(28);         // High temperature of 28C
  sensor.setLowTemp(22);          // Low temperature of 22C
  sensor.setHighHumidity(55);     // High humidity of 55%
  sensor.setLowHumidity(40);      // Low humidity of 40%

  // Configure Measurements
  sensor.setMeasurementMode(TEMP_AND_HUMID);  // Set measurements to temperature and humidity
  sensor.setRate(ONE_HZ);                     // Set measurement frequency to 1 Hz
  sensor.setTempRes(FOURTEEN_BIT);
  sensor.setHumidRes(FOURTEEN_BIT);

  //begin measuring
  sensor.triggerMeasurement();

  // Inicia la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Dirección 0x3C para pantalla de 128x32
    // Si no se puede iniciar la pantalla, detén la ejecución.
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // Iniciar conexión WiFi
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult(5000) != WL_CONNECTED) {
    Serial.println("Starting WiFi connection...");
  }

  // Inicia la conexión al servidor MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Si no estamos conectados a la red Wi-Fi, intenta reconectar
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult(5000) != WL_CONNECTED) {
      connectionAttempt++;
      if (connectionAttempt > 4) {
        connectionAttempt = 0;
      }
    } else {
      connectionAttempt = 0;
    }
  }

  float temperatura = sensor.readTemp();
  float humedad = sensor.readHumidity();

  Serial.print("Temperature (C): "); Serial.print(temperatura);
  Serial.print("\t\tHumidity (%): "); Serial.println(humedad);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Dibuja la barra de temperatura
  int maxTemp = 40;
  int minTemp = 10;
  int barHeight = ((temperatura - minTemp) / (maxTemp - minTemp)) * 28;
  barHeight = constrain(barHeight, 0, 28);
  display.drawRect(0, 0, 12, 32, WHITE); // Dibuja el borde
  display.fillRect(2, 30 - barHeight, 8, barHeight, WHITE); // Dibuja la barra de temperatura

  // Dibuja el estado de Wi-Fi
  int wifiX = SCREEN_WIDTH - 30; // La posición X base para el estado de Wi-Fi
  display.drawRect(wifiX, 0, 30, 11, WHITE); // Dibuja el borde del rectángulo
  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(wifiX + 4, 2);
    display.println("wifi");
  } else {
    for (int i = 0; i < connectionAttempt; i++) {
      display.drawPixel(wifiX + 3 + i*6, 5, WHITE);
    }
  }

  // Escribe los valores de temperatura y humedad
  display.setCursor(20, 0);
  display.println("TEMP: " + String(temperatura) + "C");
  display.setCursor(20, 11);  // Mover la fila de humedad un pixel hacia arriba
  display.println("HUME: " + String(humedad) + "%");
  
  // Escribe el tiempo transcurrido desde la última publicación MQTT exitosa
  display.setCursor(20, 23);  // Mover la fila de MQTT un pixel hacia arriba
  long secondsSinceLastPublish = (millis() - lastSuccessfulPublishMillis) / 1000;
  display.println("MQTT: Hace " + String(secondsSinceLastPublish) + " seg");

  display.display();

  // Publica los datos de temperatura y humedad en los topics correspondientes
  if (millis() - lastPublishMillis > publish_interval) {
    lastPublishMillis = millis();
    if (client.publish(temp_topic, String(temperatura).c_str(), true)) {
      lastSuccessfulPublishMillis = millis();
    }
    if (client.publish(hum_topic, String(humedad).c_str(), true)) {
      lastSuccessfulPublishMillis = millis();
    }
  }

  // Espera un segundo antes de leer de nuevo
  delay(1000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  // manejador de mensajes entrantes, si los hubiera
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("conectado");
    } else {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intento en 5 segundos");
      delay(5000);
    }
  }
}
