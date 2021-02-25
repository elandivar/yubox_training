RTC_DATA_ATTR int bootCount = 0;
int GREEN_LED_PIN = 4;
void setup(){
    delay(500);
    pinMode(GREEN_LED_PIN,OUTPUT);
    delay(500);
    digitalWrite(GREEN_LED_PIN,HIGH);
    delay(3000);
    digitalWrite(GREEN_LED_PIN,LOW);
    esp_sleep_enable_timer_wakeup(6000000);
    esp_deep_sleep_start();
}
void loop(){
}
