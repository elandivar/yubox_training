#include <Wire.h>
#include "ClosedCube_SHT31D.h"

ClosedCube_SHT31D sht3xd;

void setup()
{
	Wire.begin(25,32);

	Serial.begin(115200);

	sht3xd.begin(0x44); // I2C address: 0x44 or 0x45

	Serial.print("Cod. serial #");
	Serial.println(sht3xd.readSerialNumber());

	if (sht3xd.periodicStart(SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_10HZ) != SHT3XD_NO_ERROR)
		Serial.println("[ERROR] No se pudo inicializar el sensor");

}

void loop()
{
	printResult("Modo Periodico", sht3xd.periodicFetchData());
	delay(250);
}

void printResult(String text, SHT31D result) {
	if (result.error == SHT3XD_NO_ERROR) {
		Serial.print(text);
		Serial.print(": T=");
		Serial.print(result.t);
		Serial.print("C, RH=");
		Serial.print(result.rh);
		Serial.println("%");
	} else {
		Serial.print(text);
		Serial.print(": [ERROR] Codigo #");
		Serial.println(result.error);
	}
}
