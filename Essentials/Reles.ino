#include "PCA9536.h"
#include <Wire.h>

PCA9536 ioex;
uint32_t port = 0;

#define YUBOX_I2C_SDA               GPIO_NUM_40
#define YUBOX_I2C_SCL               GPIO_NUM_41
#define YUBOX_5V_ENABLE             GPIO_NUM_4

void setup()
{
  pinMode(YUBOX_5V_ENABLE, OUTPUT);
  digitalWrite(YUBOX_5V_ENABLE, HIGH);
  delay(1000);

  Wire.setPins(YUBOX_I2C_SDA, YUBOX_I2C_SCL);
  Wire.begin();

  ioex.setup();
}

void loop()
{
  ioex.output(port % 16);
  ++port;
  delay(1000);
}
