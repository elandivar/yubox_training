#include <Wire.h>
#include <HDC2080.h> // Esta librería funciona, tanto para el HDC2080 como para el HDC2010
#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi settings
const char* ssid = "";
const char* password = "";

// Corlysis Setting - click to the database to get those info
const char* dbName = "";
const char* dbPassword = "";
const unsigned long delayTimeMs = 20000;

// Direcciones I2C de los Sensores
#define ADDR_HDC2010 0x41
// Instancio los sensores
HDC2080 sensor(ADDR_HDC2010);
float hdc2010_data[2];

//float temperature = 0, humidity = 0;
HTTPClient http;

void setup() {
  Serial.begin(115200);
  //Wi-Fi connection
  Serial.print("Connecting to the................");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);
  //WiFi.begin(ssid);
    
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("WiFi connected.");
  Serial.print("My IP address: ");  
  Serial.println(WiFi.localIP());   

  bool status;

  // Initialize I2C communication
  sensor.begin();
  sensor.reset(); // Begin with a device reset
  sensor.setMeasurementMode(TEMP_AND_HUMID);  // Set measurements to temperature and humidity
  sensor.setRate(ONE_HZ);                     // Set measurement frequency to 1 Hz
  sensor.setTempRes(FOURTEEN_BIT);
  sensor.setHumidRes(FOURTEEN_BIT);
  //begin measuring
  sensor.triggerMeasurement();
    
  delay(100); // let sensor boot up

}


void loop() {
    // read data from sensor
  read_HDC2010(hdc2010_data);
    
  sendData(hdc2010_data[0], hdc2010_data[1]);
  delay(delayTimeMs);
}


void sendData(float temperature, float humidity) {
    static long counter = 0;
    
    char payloadStr[150];
    sprintf(payloadStr, "yubox_temp temperature=%d.%02d,humidity=%d.%02d", (int)temperature, (int)abs(temperature*100)%100, 
    (int)humidity, (int)abs(humidity*100)%100);
    Serial.println(payloadStr);
    
    char corlysisUrl[200];
    sprintf(corlysisUrl, "http://corlysis.com:8087/write?db=%s&u=token&p=%s", dbName, dbPassword);
    http.begin(corlysisUrl);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(payloadStr);
    Serial.print("http result:");
    Serial.println(httpCode);
    http.writeToStream(&Serial);
    http.end();

    if(httpCode == 204) {
        counter = 0;
        Serial.println("Data successfully sent.");
    }else{
        Serial.println("Data were not sent. Check network connection.");
    }
    Serial.println("");  
}

void read_HDC2010(float hdc2010_data[]) {
  //Serial.print("Temperature (C): "); Serial.print(sensor.readTemp());
  //Serial.print("\t\tHumidity (%): "); Serial.println(sensor.readHumidity()); 
  hdc2010_data[0] = sensor.readTemp();
  hdc2010_data[1] = sensor.readHumidity();
  if(hdc2010_data[0]<=(float)(-38)) {
    Serial.println("SE SALTO DE LECTURA DEL HDC2010!");
    hdc2010_data[0] = NULL;
    hdc2010_data[1] = NULL;
  }
}
