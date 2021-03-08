#include <Wire.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT; 
String msg;
unsigned long previousMillis = 0;
const long interval = 10000;

void setup() {
  Serial.begin(115200);

  SerialBT.begin("CursoYubox"); // Nombre del dispositivo bluetooth 
  Serial.println("Dispositivo iniciado");
}

void loop() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      SerialBT.print("Hola, soy Yubox \n");
    }

    
   if (SerialBT.available()){
    char inchar = SerialBT.read();
    if (inchar != '\n') {
      msg += String(inchar);
    } else {
      delay(500);
      msg = "";
    }
  } 

  Serial.print(msg);
  msg = "";
}
