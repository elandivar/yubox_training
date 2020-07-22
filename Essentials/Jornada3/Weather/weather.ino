#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
 
const char* ssid = "XXXXXX";
const char* password = "XXXXXX";
const String lat="-2.203816";
const String lon="-79.897453";
const String apikey = "XXXXXXXXXXXXXXXXXXXXXXXX"; // API KEY de https://openweathermap.org/
 
const String weather_url = "http://api.openweathermap.org/data/2.5/onecall?lat=" + lat + "&lon=" + lon + "&exclude=current,hourly&appid=" + apikey;

void setup() {
 
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
 
}
 
void loop() {
  
    HTTPClient http;
    http.begin(weather_url); //Specify the URL
    int httpCode = http.GET();  //Make the request
 
    if (httpCode > 0) { //Check for the returning code

        DynamicJsonDocument doc(2048);
        deserializeJson(doc, http.getStream());
        
        Serial.println();
        Serial.println("Pronóstico del tiempo");
        Serial.println("---------------------");
        Serial.println();

        Serial.println("Hoy al medio día: ");
        Serial.print("   Temp. mínima: "); Serial.print(doc["daily"][0]["temp"]["min"].as<float>()/10); Serial.println("°C");
        Serial.print("   Temp. máxima: "); Serial.print(doc["daily"][0]["temp"]["max"].as<float>()/10); Serial.println("°C");
        Serial.print("   Estado del tiempo: "); Serial.println(weather_cond(doc["daily"][0]["weather"][0]["id"].as<int>())); 
        Serial.print("   Nubosidad: "); Serial.print(doc["daily"][0]["clouds"].as<int>()); Serial.println("%");
        Serial.println();

        Serial.println("Mañana: ");
        Serial.print("   Temp. mínima: "); Serial.print(doc["daily"][1]["temp"]["min"].as<float>()/10); Serial.println("°C");
        Serial.print("   Temp. máxima: "); Serial.print(doc["daily"][1]["temp"]["max"].as<float>()/10); Serial.println("°C");
        Serial.print("   Estado del tiempo: "); Serial.println(weather_cond(doc["daily"][1]["weather"][0]["id"].as<int>())); 
        Serial.print("   Nubosidad: "); Serial.print(doc["daily"][1]["clouds"].as<int>()); Serial.println("%");
        Serial.println();
        
    } else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
 
   delay(30000);
 
}

char * weather_cond(int wcode) {

   if( (wcode>=200) && (wcode<300) ) {
      return "tormenta eléctrica";
   } else if( (wcode>=300) && (wcode<400) ) {
      return "llovizna";
   } else if( (wcode>=500) && (wcode<600) ) {
      return "lluvia";
   } else if( (wcode>=600) && (wcode<700) ) {
      return "nieve";
   } else if( wcode==800 ) {
      return "despejado";
   } else if( wcode==801 ) {
      return "ligeramente nublado";
   } else if( (wcode>=802) && (wcode<803) ) {
      return "parcialmente nublado";
   } else if( wcode>=804 ) {
      return "nublado";
   } else {
      return "desconocido";
   }
}

