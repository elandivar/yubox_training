#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // ancho de la pantalla OLED
#define SCREEN_HEIGHT 32 // altura de la pantalla OLED

// Declara una instancia del objeto SSD1306 llamada display.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Dirección 0x3C para pantalla de 128x32
    // Si no se puede iniciar la pantalla, detén la ejecución.
    for (;;);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  float temperatura = 25.2; // Aquí debes obtener tu valor real de temperatura
  float humedad = 60.8;     // Aquí debes obtener tu valor real de humedad

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Dibuja el icono del termómetro
  // Aquí debes dibujar o cargar tu icono de termómetro
  // Por simplicidad, dibujaré un rectángulo como lugar reservado para tu icono
  display.drawRect(0, 0, 10, 30, WHITE);

  // Escribe los valores de temperatura y humedad
  display.setCursor(20, 0);
  display.println("TEMP: " + String(temperatura) + "C");
  display.setCursor(20, 15);
  display.println("HUME: " + String(humedad) + "%");

  display.display();
  delay(1000);
}
