#include <Arduino.h>

#if CONFIG_IDF_TARGET_ESP32

// NOTA: GPIO_NUM_34, GPIO_NUM_36, GPIO_NUM_39 es sólo input! Verificar...
#define YUBOX_MOTOR_IN_A_1  GPIO_NUM_27
#define YUBOX_MOTOR_IN_A_2  GPIO_NUM_34
#define YUBOX_MOTOR_PWM_A   GPIO_NUM_36

#define YUBOX_MOTOR_IN_B_1  GPIO_NUM_16
#define YUBOX_MOTOR_IN_B_2  GPIO_NUM_17
#define YUBOX_MOTOR_PWM_B   GPIO_NUM_39

#elif CONFIG_IDF_TARGET_ESP32S2

#define YUBOX_MOTOR_IN_A_1  GPIO_NUM_10
#define YUBOX_MOTOR_IN_A_2  GPIO_NUM_12
#define YUBOX_MOTOR_PWM_A   GPIO_NUM_1

#define YUBOX_MOTOR_IN_B_1  GPIO_NUM_11
#define YUBOX_MOTOR_IN_B_2  GPIO_NUM_13
#define YUBOX_MOTOR_PWM_B   GPIO_NUM_2

#else
#error Pines de control no definidos para board objetivo!
#endif

#define LEDC_BASE_FREQ      5000

// Bits de resolución para valor a escribir con ledcWrite
#define RESOLUTION          12

// Valor máximo del DUTY a programar en la tarjeta, calculado a partir de RESOLUTION
unsigned int g_maxDuty = 0;

void iniciarMotor(uint8_t chan, uint8_t gpio_fwd, uint8_t gpio_rev, uint8_t pwm)
{
  Serial.printf("INICIANDO en chan=%u FWD=%u REV=%u con PWM=%u\r\n",
    chan, gpio_fwd, gpio_rev, pwm);
  ledcSetup(chan, LEDC_BASE_FREQ, RESOLUTION);
  ledcAttachPin(pwm, chan);
  ledcWrite(chan, 0);

  pinMode(gpio_fwd, OUTPUT);
  digitalWrite(gpio_fwd, LOW);

  pinMode(gpio_rev, OUTPUT);
  digitalWrite(gpio_rev, LOW);
}

void setup()
{
  // La siguiente demora es sólo para comodidad de desarrollo para enchufar el USB
  // y verlo en gtkterm. No es en lo absoluto necesaria como algoritmo requerido.
  delay(3000);
  Serial.begin(115200);

  Serial.printf("Frecuencia base PWM es %u\r\n", LEDC_BASE_FREQ);
  
  g_maxDuty = (1U << RESOLUTION) - 1;
  Serial.printf("Para RESOLUTION=%u el duty máximo es %u\r\n", RESOLUTION, g_maxDuty);

  // Programar pines de las tarjetas para el control
  iniciarMotor(0, YUBOX_MOTOR_IN_A_1, YUBOX_MOTOR_IN_A_2, YUBOX_MOTOR_PWM_A);
  iniciarMotor(1, YUBOX_MOTOR_IN_B_1, YUBOX_MOTOR_IN_B_2, YUBOX_MOTOR_PWM_B);
}

void pruebaTodaPotencia(uint8_t chan, uint8_t gpio_fwd, uint8_t gpio_rev)
{
  Serial.printf("PRUEBA POTENCIA INMEDIATA: chan=%u FWD=%u REV=%u\r\n", chan, gpio_fwd, gpio_rev);

  // Toda potencia en PWM
  ledcWrite(chan, g_maxDuty);

  // Encender la dirección hacia DELANTE...
  digitalWrite(gpio_fwd, HIGH);
  digitalWrite(gpio_rev, LOW);

  Serial.printf("- MOTOR %u DEBERÍA ESTAR ENCENDIDO A MÁXIMA POTENCIA, VERIFICAR...\r\n", chan);
}

void apagarMotor(uint8_t chan, uint8_t gpio_fwd, uint8_t gpio_rev)
{
  // Apagar completamente PWM
  ledcWrite(chan, 0);
  
  // Apagar la dirección hacia DELANTE...
  digitalWrite(gpio_fwd, LOW);
  digitalWrite(gpio_rev, LOW);

  Serial.printf("- MOTOR %u DEBERÍA ESTAR APAGADO, VERIFICAR...\r\n", chan);
}

void pruebaRampaIncremento(uint8_t chan, uint8_t gpio_fwd, uint8_t gpio_rev)
{
  Serial.printf("PRUEBA RAMPA INCREMENTO: chan=%u FWD=%u REV=%u\r\n", chan, gpio_fwd, gpio_rev);

  unsigned int d = 5000 / (g_maxDuty + 1);
  if (d <= 0) d = 1;
  Serial.printf("- esperando %u ms entre cambios de duty\r\n", d);

  // Encender la dirección hacia DELANTE...
  digitalWrite(gpio_fwd, HIGH);
  digitalWrite(gpio_rev, LOW);

  for (int duty = 0; duty <= g_maxDuty; duty++) {
    ledcWrite(chan, duty);
    delay(d);
  }

  Serial.printf("- MOTOR %u DEBERÍA ESTAR ENCENDIDO A MÁXIMA POTENCIA, VERIFICAR...\r\n", chan);
  delay(1000);
}

void pruebaRampaDecremento(uint8_t chan, uint8_t gpio_fwd, uint8_t gpio_rev)
{
  Serial.printf("PRUEBA RAMPA DECREMENTO: chan=%u FWD=%u REV=%u\r\n", chan, gpio_fwd, gpio_rev);

  unsigned int d = 5000 / (g_maxDuty + 1);
  if (d <= 0) d = 1;
  Serial.printf("- esperando %u ms entre cambios de duty\r\n", d);

  // Encender la dirección hacia DELANTE...
  digitalWrite(gpio_fwd, HIGH);
  digitalWrite(gpio_rev, LOW);

  for (int duty = g_maxDuty; duty >= 0; duty--) {
    ledcWrite(chan, duty);
    delay(d);
  }

  apagarMotor(chan, gpio_fwd, gpio_rev);
  delay(1000);
}

void loop()
{
  Serial.println("PRUEBA AL DERECHO");
  
  // Probar primer motor, rampa ascendente y luego descendente
  pruebaRampaIncremento(0, YUBOX_MOTOR_IN_A_1, YUBOX_MOTOR_IN_A_2);
  pruebaRampaDecremento(0, YUBOX_MOTOR_IN_A_1, YUBOX_MOTOR_IN_A_2);

  // Probar segundo motor, rampa ascedente y luego descendente
  pruebaRampaIncremento(1, YUBOX_MOTOR_IN_B_1, YUBOX_MOTOR_IN_B_2);
  pruebaRampaDecremento(1, YUBOX_MOTOR_IN_B_1, YUBOX_MOTOR_IN_B_2);

  // Probar primer motor, encendido inmediato
  pruebaTodaPotencia(0, YUBOX_MOTOR_IN_A_1, YUBOX_MOTOR_IN_A_2);
  delay(5000);
  apagarMotor(0, YUBOX_MOTOR_IN_A_1, YUBOX_MOTOR_IN_A_2);
  delay(1000);

  // Probar segundo motor, encendido inmediato
  pruebaTodaPotencia(1, YUBOX_MOTOR_IN_B_1, YUBOX_MOTOR_IN_B_2);
  delay(5000);
  apagarMotor(1, YUBOX_MOTOR_IN_B_1, YUBOX_MOTOR_IN_B_2);
  delay(1000);

  Serial.println("PRUEBA AL REVÉS");

  // Probar primer motor, rampa ascendente y luego descendente
  pruebaRampaIncremento(0, YUBOX_MOTOR_IN_A_2, YUBOX_MOTOR_IN_A_1);
  pruebaRampaDecremento(0, YUBOX_MOTOR_IN_A_2, YUBOX_MOTOR_IN_A_1);

  // Probar segundo motor, rampa ascedente y luego descendente
  pruebaRampaIncremento(1, YUBOX_MOTOR_IN_B_2, YUBOX_MOTOR_IN_B_1);
  pruebaRampaDecremento(1, YUBOX_MOTOR_IN_B_2, YUBOX_MOTOR_IN_B_1);

  // Probar primer motor, encendido inmediato
  pruebaTodaPotencia(0, YUBOX_MOTOR_IN_A_2, YUBOX_MOTOR_IN_A_1);
  delay(5000);
  apagarMotor(0, YUBOX_MOTOR_IN_A_2, YUBOX_MOTOR_IN_A_1);
  delay(1000);

  // Probar segundo motor, encendido inmediato
  pruebaTodaPotencia(1, YUBOX_MOTOR_IN_B_2, YUBOX_MOTOR_IN_B_1);
  delay(5000);
  apagarMotor(1, YUBOX_MOTOR_IN_B_2, YUBOX_MOTOR_IN_B_1);
  delay(1000);
}
