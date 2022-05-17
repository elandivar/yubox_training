// Yubox Companion
// Prueba LEDs on-board
// Created by: Edgar Landivar
// Usted es libre de copiar y distribuir
// www.yubox.com
// Nota: hay un error introducido deliveradamente en este ejercicio. Por favor, encuéntrelo y corríjalo.

#define LED1 4

void setup() {
  pinMode(LED, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED, HIGH);   
  delay(1000);     
  digitalWrite(LED, LOW);   
  delay(1000);            
}
