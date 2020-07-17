/***********************************************************************
*
***********************************************************************/

#include "Nextion.h"

// Page 0
NexNumber temperatura0 = NexNumber(0, 1, "temperatura0");
NexText tiempo0        = NexText(0, 5, "tiempo0");
NexText pronostico0    = NexText(0, 6, "pronostico0");
NexDSButton boton0     = NexDSButton(0, 4, "boton0");

NexTouch *nex_listen_list[] = {&boton0, NULL};

void setup() {
   Serial.begin(115200);
   Serial2.begin(115200, SERIAL_8N1, 25, 32); 
   nexInit(0);

   //Botones
   boton0.attachPush(boton_presionado, &boton0);
   pronostico0.setText("Pronóstico hoy");
}

void loop() {

      nexLoop(nex_listen_list);
      temperatura0.setValue(280);
      tiempo0.setText("Nublado");
}

void boton_presionado(void *ptr) {
    uint32_t dual_state;
    boton0.getValue(&dual_state);
    if(dual_state!=0) {
        boton0.setText("Ver Mañana");
        pronostico0.setText("Pronóstico hoy");
        Serial.println("boton presionado");
        Serial.println("Estado HOY");
    } else {
        boton0.setText("Ver Hoy");
        pronostico0.setText("Pronóstico mañana");
        Serial.println("boton presionado");
        Serial.println("Estado MAÑANA");
    }
}
