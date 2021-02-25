RTC_DATA_ATTR int bootCount = 0;
int GREEN_LED_PIN = 4;
void setup(){
    delay(500);
    bootCount++;
    Serial.begin(115200);
    pinMode(GREEN_LED_PIN,OUTPUT);
    delay(500);
    digitalWrite(GREEN_LED_PIN,HIGH);
    delay(3000);
    digitalWrite(GREEN_LED_PIN,LOW);
    Serial.print("Numero de inicios: ");
    Serial.println(bootCount);
    esp_sleep_enable_timer_wakeup(6000000);
    esp_deep_sleep_start();
}
void loop(){
}
