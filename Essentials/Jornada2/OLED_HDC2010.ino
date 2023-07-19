#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HDC2080.h>
#include <WiFi.h>

#define ADDR 0x41
HDC2080 sensor(ADDR);

#define SCREEN_WIDTH 128 // ancho de la pantalla OLED
#define SCREEN_HEIGHT 32 // altura de la pantalla OLED

// Declara una instancia del objeto SSD1306 llamada display.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Define las credenciales de Wi-Fi
const char* ssid     = "ssid";
const char* password = "password";

// Contador para los puntos de intento de conexión
int connectionAttempt = 0;

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
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Starting WiFi connection...");
  }
}

void loop() {
  // Si no estamos conectados a la red Wi-Fi, intenta reconectar
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, password);
    connectionAttempt++;
    if (connectionAttempt > 4) {
      connectionAttempt = 0;
    }
  } else {
    connectionAttempt = 0;
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
  display.setCursor(20, 15);
  display.println("HUME: " + String(humedad) + "%");

  display.display();

  // Espera un segundo antes de leer de nuevo
  delay(1000);
}
