// Todos los sensores ejercitados usan I2C
#include <Wire.h>
// Biblioteca para el sensor HDC2010/HDC2080 de temperatura/humedad
#include <HDC2080.h>

// Direcciones I2C de los sensores
#define ADDR_HDC2010 0x41

// Instancias de los sensores
HDC2080 hdc2010_sensor(ADDR_HDC2010);

void setup()
{
  delay(3 * 1000);

  Serial.begin(115200);
  while(!Serial);

  Wire.begin();

  setup_hdc2010_sensor();

  delay(500);
}

void reset_hdc2010_sensor()
{
  hdc2010_sensor.reset();
  hdc2010_sensor.setMeasurementMode(TEMP_AND_HUMID);  // Set measurements to temperature and humidity
  hdc2010_sensor.setRate(ONE_HZ);                     // Set measurement frequency to 1 Hz
  hdc2010_sensor.setTempRes(FOURTEEN_BIT);
  hdc2010_sensor.setHumidRes(FOURTEEN_BIT);
  
  // Se inicia la medición
  hdc2010_sensor.triggerMeasurement();
}

void setup_hdc2010_sensor()
{
  hdc2010_sensor.begin();
  reset_hdc2010_sensor();
}

void loop()
{
  Serial.println("\n\nLeyendo sensores...");

  Serial.println("\nSensor HDC2010:");

  float t, h;
  t = hdc2010_sensor.readTemp();
  h = hdc2010_sensor.readHumidity();
  if (t == -40.0f && h == 0.0f) {
    Serial.println("ERROR");
    reset_hdc2010_sensor();
  } else {
    Serial.print("Temperatura (C): "); Serial.println(t);
    Serial.print("Humedad (%): "); Serial.println(h);
  }

  delay(5000);
}
