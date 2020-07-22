#define PULS 36
#define LED 4

void setup() {

  pinMode(PULS, INPUT);
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200);
}

void loop() { 
  if (digitalRead(PULS) == LOW) {       
    digitalWrite(LED, HIGH);
    Serial.println("Aplastado"); 
    delay(500);
  } else {
    digitalWrite(LED, LOW);
    Serial.println("NO Aplastado"); 
  }
}
