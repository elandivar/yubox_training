#include "Nextion.h"

// Page 0
NexText t0        = NexText(0, 3, "t0");
NexDSButton b0    = NexDSButton(0, 2, "b0");

NexTouch *nex_listen_list[] = {&b0, NULL};

void setup() {
   Serial.begin(115200);
   Serial2.begin(250000, SERIAL_8N1, 25, 32); 
   nexInit(0);

   //Botones
   b0.attachPush(boton_presionado, &b0);
   t0.setText("Hola, presiona el boton");
}

void loop() {
      nexLoop(nex_listen_list);
}

void boton_presionado(void *ptr) {
    t0.setText("Bien!!!!");
}
