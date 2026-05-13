#include <Arduino.h>

extern "C" {
  #include "app.h"
}

#define BT_BAUDRATE 9600

void setup() {
  Serial.begin(115200);
  Serial1.begin(BT_BAUDRATE);

  app_init();
}

void loop() {
  app_loop();
}