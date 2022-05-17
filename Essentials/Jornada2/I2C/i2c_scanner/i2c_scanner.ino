#include <Wire.h>

void setup() {
  // delay(500); // Usar en caso de problemas
  Serial.begin (9600);
  Serial.println ("I2C scanner. Escaneando dispositivos I2C...");
  byte count = 0;
  
  //Wire.begin(21,22);
  Wire.begin(32,25);

  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  // maybe unneeded?
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
}  // end of setup

void loop() {}
