// Todos los sensores ejercitados usan I2C
#include <Wire.h>
// Biblioteca para el sensor HDC2010/HDC2080 de temperatura/humedad
#include <HDC2080.h>
// Biblioteca para el sensor MPL3115A2 de presión/temperatura/altitud
#include <Adafruit_MPL3115A2.h>
// Biblioteca para el sensor RPR-0521RS de proximidad/luminosidad
#include <RPR-0521RS.h>

// Direcciones I2C de los sensores
#define ADDR_HDC2010 0x41

// Instancias de los sensores
HDC2080 hdc2010_sensor(ADDR_HDC2010);
Adafruit_MPL3115A2 mpl3115a2_sensor;
RPR0521RS rpr0521rs_sensor;

void setup()
{
  delay(3 * 1000);

  Serial.begin(115200);
  while(!Serial);

  Wire.begin();

  setup_hdc2010_sensor();
  setup_mpl3115a2_sensor();
  setup_rpr0521rs_sensor();

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

void setup_mpl3115a2_sensor()
{
  if (!mpl3115a2_sensor.begin()) {
    Serial.println("ERR: no se encuentra sensor MPL3115A2!");
  }
}

void setup_rpr0521rs_sensor()
{
  if (rpr0521rs_sensor.init()) {
    Serial.println("ERR: no se puede iniciar sensor RPR0521RS!");
  }
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

  Serial.println("\nSensor MPL3115A2:");
  Serial.print("Temperatura (C): "); Serial.println(mpl3115a2_sensor.getTemperature());
  Serial.print("Presión (pa): "); Serial.println(mpl3115a2_sensor.getPressure());

  Serial.println("\nSensor RPR0521RS:");
  unsigned short ps_val;
  float als_val;
  byte rc = rpr0521rs_sensor.get_psalsval(&ps_val, &als_val);
  if (rc != 0) {
    Serial.println("ERROR");
  } else {
    Serial.print("Proximidad (count): "); Serial.println(ps_val);
    Serial.print("Luz ambiental (lx): "); Serial.println(als_val);
  }
  delay(5000);
}
