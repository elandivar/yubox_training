/* Yubox Node
   Prueba sensor de:
   - Sensor BMP-280 
   - Módulo de calidad de aire, equipado con:
        - Sensor CCS811
        - Sensor HDC2010
   Estos sensores se encuentran conectados al bus I2C
   Created by: Edgar Landivar
   Usted es libre de copiar y distribuir
   www.yubox.com
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "SparkFunCCS811.h"
#include <HDC2080.h> // Esta librería funciona, tanto para el HDC2080 como para el HDC2010
#include "SSD1306Wire.h" // Librería para salida en pantalla OLED 128
#include <WiFi.h>

// Direcciones I2C de los Sensores
#define ADDR_CCS811 0x5B 
#define ADDR_HDC2010 0x41
// Direccion LCD OLED 128
#define ADDR_OLED 0x3c

#define PCB_SWITCH_1 36
#define SEALEVELPRESSURE_HPA (1012.00)
#define HDC2010_TEMP_OFFSET -3.4
#define LOWPASS_ALPHA 0.28
#define LOWPASS_ALPHA2 0.95
#define LOWPASS_ALPHA3 0.98
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_GRAPH_MARGIN_L 2
#define OLED_GRAPH_MARGIN_R 24
#define OLED_GRAPH_MARGIN_T 4
#define OLED_GRAPH_MARGIN_B 4
#define TIME_BETWEEN_SLIDES 12

// Instancio los sensores
CCS811 mySensor(ADDR_CCS811);
HDC2080 sensor(ADDR_HDC2010);

SSD1306Wire  display(ADDR_OLED, 21, 22);

unsigned long delayTime;
float temp, pres;
bool status;
float temperature = 0, humidity = 0;

float hdc2010_data[2];
float ccs811_data[2];

int graph_ymax = 35;
int graph_ymin = 15;

float hdc2010_temp_log[128];
float hdc2010_hum_log[128];
float hdc2010_hum_lowpass_log[128];
float ccs811_co2_log[128];
float ccs811_co2_lowpass_log[128];
float ccs811_tvoc_log[128];
float ccs811_tvoc_lowpass_log[128];
float avg_temp_lowpass_log[128];
float esp32_hall_log[128];
float graphdata[256];
char graphlabel[512];
int graphid=1;
int init_time;
float co2_filtered_last=0;
float co2_filtered_curr=0;
float tvoc_filtered_last=0;
float tvoc_filtered_curr=0;
float avgtemp_filtered_last=0;
float avgtemp_filtered_curr=0;
float avgtemp_corrected=0;
float hdc2010_hum_lowpass_last=0;

int logoyubox[7][10][2] = 
{{{60,48},{53,59},{46,59},{46,58},{52,46},{60,48},{60,48}},
{{53,44},{58,32},{69,35},{61,46},{53,44},{53,44}},
{{50,14},{51,6},{65,6},{62,16},{50,14},{50,14}},
{{61,19},{56,30},{48,28},{49,17},{61,19},{61,19}},
{{59,30},{64,19},{77,22},{70,33},{59,30},{59,30}},
{{88,6},{69,6},{68,9},{68,9},{65,17},{79,19},{86,9},{86,9},{88,6},{88,6}},
{{78,17},{68,16},{71,9},{71,9},{84,9},{78,17},{78,17}}};

void setup() {
  pinMode(PCB_SWITCH_1, INPUT);
  Serial.begin(115200);
  while(!Serial);
  wakeUpCCS811();

  display.init();
  // display.flipScreenVertically();
  display.setContrast(255);
  delay(1000);
  display.clear();

  mySensor.begin();

  // Initialize I2C communication
  sensor.begin();
  sensor.reset(); // Begin with a device reset
  sensor.setMeasurementMode(TEMP_AND_HUMID);  // Set measurements to temperature and humidity
  sensor.setRate(ONE_HZ);                     // Set measurement frequency to 1 Hz
  sensor.setTempRes(FOURTEEN_BIT);
  sensor.setHumidRes(FOURTEEN_BIT);
  //begin measuring
  sensor.triggerMeasurement();

  drawLineShow();
  draw_logo(0,0,22);
  delay(1500);
  for (int i=0; i<100; i++) {
    draw_logo(i*2,0,0);
  }
}

/*******************************************
 ****          LOOP PRINCIPAL           ****
 *******************************************/
 
void loop() {
  
  if (digitalRead(PCB_SWITCH_1) == LOW) {       
    Serial.println("Aplastado SWITCH 1"); 
    graphid++;
    init_time = millis();
    delay(140);
  } 
  
  read_HDC2010(hdc2010_data);
  read_CCS811(ccs811_data);

  // Dibujo la temperatura. Voy a asumir un rango de entre 15 y 35 por ahora. Luego lo haré más dinámico.
  // Eso ya está definido arriba con las variables   int graph_ymax = 35; y int graph_ymin = 15;
  
  memcpy(hdc2010_temp_log, &hdc2010_temp_log[1], sizeof(hdc2010_temp_log) - sizeof(float));
  hdc2010_temp_log[127] = hdc2010_data[0];  
  
  memcpy(hdc2010_hum_log, &hdc2010_hum_log[1], sizeof(hdc2010_hum_log) - sizeof(float));
  hdc2010_hum_log[127] = hdc2010_data[1];

  if(hdc2010_data[1]==NULL) { // Es por que a veces se salta las lecturas el sensor
    hdc2010_hum_lowpass_log[127] = NULL;
  } else {
    hdc2010_hum_lowpass_last = filter_sample(hdc2010_hum_lowpass_log, 128, hdc2010_data[1], LOWPASS_ALPHA2, hdc2010_hum_lowpass_last);
  }
    
  memcpy(ccs811_co2_log, &ccs811_co2_log[1], sizeof(ccs811_co2_log) - sizeof(float));
  ccs811_co2_log[127] = ccs811_data[0];
  // Un experimento de filtro pasa bajas
  memcpy(ccs811_co2_lowpass_log, &ccs811_co2_lowpass_log[1], sizeof(ccs811_co2_lowpass_log) - sizeof(float));
  co2_filtered_curr = (float)LOWPASS_ALPHA * ccs811_data[0] + (1-(float)LOWPASS_ALPHA) * co2_filtered_last;
  ccs811_co2_lowpass_log[127] = co2_filtered_curr;
  co2_filtered_last = co2_filtered_curr;
  //
  memcpy(ccs811_tvoc_log, &ccs811_tvoc_log[1], sizeof(ccs811_tvoc_log) - sizeof(float));
  ccs811_tvoc_log[127] = ccs811_data[1];
  // Un experimento de filtro pasa bajas
  memcpy(ccs811_tvoc_lowpass_log, &ccs811_tvoc_lowpass_log[1], sizeof(ccs811_tvoc_lowpass_log) - sizeof(float));
  tvoc_filtered_curr = (float)LOWPASS_ALPHA * ccs811_data[1] + (1-(float)LOWPASS_ALPHA) * tvoc_filtered_last;
  ccs811_tvoc_lowpass_log[127] = tvoc_filtered_curr;
  tvoc_filtered_last = tvoc_filtered_curr;
  
  memcpy(esp32_hall_log, &esp32_hall_log[1], sizeof(esp32_hall_log) - sizeof(float));
  esp32_hall_log[127] = (float)hallRead();

  Serial.print("Temp Correg (");
  Serial.print(hdc2010_data[0]);
  Serial.print(",");
  if(millis()-init_time>((int)TIME_BETWEEN_SLIDES*1000)) { 
    init_time = millis();
    graphid++;
  }
  
  if(graphid>5) graphid=1;
  if(graphid<1) graphid=5;
  
  switch(graphid) {
    case 1:
      Serial.println("HDC2010 HUME.");
      draw_graph(hdc2010_hum_lowpass_log, "HUMED", "%", 0.3);
      break;
    case 2:
      // CO2 FILTRADO (FILTRO PASA BAJAS)
      Serial.println("CCS811 CO2"); 
      draw_graph(ccs811_co2_lowpass_log, "CO2", "", 0.3);
      break;
    case 3:
      Serial.println("CCS811 TVOC");
      draw_graph(ccs811_tvoc_lowpass_log, "TVOC", "", 0.3);
      break;
    // A partir de aquí no muestro las estadísticas
    case 4:
      Serial.println("HDC2010 TEMP.");
      draw_graph(hdc2010_temp_log, "TEMP", "C", 0);
      break;
    case 5:
      Serial.println("HDC2010 HUME.");
      draw_graph(hdc2010_hum_log, "HUMED", "%", 0.3);
      break;
    case 8:
      Serial.println("CCS811 CO2");
      draw_graph(ccs811_co2_log, "CO2", "", 0);
      break;
    case 9:
      Serial.println("CCS811 TVOC");
      draw_graph(ccs811_tvoc_log, "TVOC", "", 0);
      break;
    case 12:
      Serial.println("HALL SENSOR");
      draw_graph(esp32_hall_log, "HALL", "", 0);
      break;
    default:
      draw_graph(hdc2010_temp_log, "TEMP", "C", 0);
  }
  
  // Wait 1 second for the next reading
  delay(5500);
  
}

/*******************************************
 ****         FUNCIONES LOCALES         ****
 *******************************************/

float filter_sample(float *arr_stats, int arr_size, float new_sample, float alpha, float last_filtered_value) {
  float curr_filtered_value;
  //memcpy(arr_stats, &arr_stats[1], sizeof(arr_stats) - sizeof(float));
  memcpy(arr_stats, &arr_stats[1], (arr_size-1)*sizeof(float));
  curr_filtered_value = alpha * new_sample + (1-alpha) * last_filtered_value;
  arr_stats[127] = curr_filtered_value;
  return curr_filtered_value;
}
 
void wakeUpCCS811() {
  pinMode(19, OUTPUT);
  digitalWrite(19, LOW);
}

void read_HDC2010(float hdc2010_data[]) {
  //Serial.print("Temperature (C): "); Serial.print(sensor.readTemp());
  //Serial.print("\t\tHumidity (%): "); Serial.println(sensor.readHumidity()); 
  hdc2010_data[0] = sensor.readTemp();
  hdc2010_data[1] = sensor.readHumidity();
  if(hdc2010_data[0]<=(float)(-38)) {
    Serial.println("SE SALTO DE LECTURA DEL HDC2010!");
    hdc2010_data[0] = NULL;
    hdc2010_data[1] = NULL;
  }
}

void read_CCS811(float ccs811_data[]) {
      //Check to see if data is ready with .dataAvailable()
  if (mySensor.dataAvailable())
  {
    //If so, have the sensor read and calculate the results.
    //Get them later
    mySensor.readAlgorithmResults();

    ccs811_data[0] = mySensor.getCO2();
    ccs811_data[1] = mySensor.getTVOC();

  }
}

void draw_graph(float graphdata[], char graphlabel[], char unidadmedida[], float extraweightY) {
  float min_y = NULL;
  float max_y = NULL;
  float escala_x, escala_y;
  String reading = "";
  char tick_text[64];
  float est_dev_normalized;
  float tick_y, tick_width;
  float yaxis_width = 0;
  display.clear();
  
  for(int i=0; i<128; i++) {
    if((min_y==NULL) && (graphdata[i] !=NULL)) min_y = graphdata[i];
    if((max_y==NULL) && (graphdata[i] !=NULL)) min_y = graphdata[i];
    if((graphdata[i] !=NULL) && graphdata[i]<min_y) min_y = graphdata[i];
    if((graphdata[i] !=NULL) && graphdata[i]>max_y) max_y = graphdata[i];
  }
  if(min_y==NULL || max_y==NULL) {
    // NO HAY DATA PARA MOSTRAR
  }
  
  if((max_y-min_y)>0) escala_y = ((float)OLED_HEIGHT-(float)OLED_GRAPH_MARGIN_T-(float)OLED_GRAPH_MARGIN_B)/(max_y-min_y); 
  else escala_y = 1;

  if(extraweightY>0) escala_y = escala_y*(1-extraweightY);

  /*
  est_dev_normalized = getStdDev(graphdata, 128);
  if(est_dev_normalized>1.9) escala_y = escala_y*0.75;
  */
  
  Serial.print("Escala: ");
  Serial.print(escala_y);
  Serial.print(", Min: ");
  Serial.print(min_y);
  Serial.print(", Max: ");
  Serial.print(max_y);
  Serial.print(", N.E.D: ");
  Serial.print(est_dev_normalized);
  Serial.print(", Coord Y ultima: ");
  Serial.println(escala_y*graphdata[127]);

  // Ticks
  for(int i=0; i<3; i++) {
    if(((i+1)*((max_y-min_y)/3)+min_y)<100) {
      sprintf(tick_text, "%.1f", (float)((i+1)*((max_y-min_y)/3)+min_y));
    } else {
      sprintf(tick_text, "%.0f", (float)((i+1)*((max_y-min_y)/3)+min_y));
    }
    tick_width = display.getStringWidth((String)tick_text);

    if(yaxis_width<(tick_width+8)) yaxis_width=tick_width+8;
  }

  for(int i=0; i<3; i++) {
    if(((i+1)*((max_y-min_y)/3)+min_y)<100) {
      sprintf(tick_text, "%.1f", (float)((i+1)*((max_y-min_y)/3)+min_y));
    } else {
      sprintf(tick_text, "%.0f", (float)((i+1)*((max_y-min_y)/3)+min_y));
    }
    tick_y = OLED_HEIGHT-(escala_y*((i+1)*((max_y-min_y)/3) ) + (float)OLED_GRAPH_MARGIN_B );
    display.drawLine((float)OLED_WIDTH-yaxis_width, tick_y, (float)OLED_WIDTH-yaxis_width+4, tick_y);
    display.setFont(ArialMT_Plain_10);
    display.setColor(WHITE);
    display.drawString((float)OLED_WIDTH-yaxis_width+3, tick_y+1, tick_text);
  }
  
  // Dibujo eje y (vertical)
  display.drawLine((float)OLED_WIDTH-yaxis_width, 10, (float)OLED_WIDTH-yaxis_width, 62);

  Serial.print("Tick Width:");
  Serial.println(tick_width);

  escala_x = ((float)OLED_WIDTH-(float)OLED_GRAPH_MARGIN_L-(float)tick_width-12)/128; 

  for(int i=0; i<127; i++) {
    if(graphdata[i]!=NULL && graphdata[i+1]==NULL) graphdata[i+1] = graphdata[i];
    if(graphdata[i]!=NULL && graphdata[i+1]!=NULL) {
      display.drawLine((i)*escala_x+OLED_GRAPH_MARGIN_L, OLED_HEIGHT-(escala_y*(graphdata[i]-min_y)+OLED_GRAPH_MARGIN_B), (i+1)*escala_x+OLED_GRAPH_MARGIN_L, OLED_HEIGHT-(escala_y*(graphdata[i+1]-min_y)+OLED_GRAPH_MARGIN_B));
    }
  }

  reading = (String)graphlabel + ": " + (int)graphdata[127] + (String)unidadmedida;
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(BLACK);
  display.fillRect(4,1,display.getStringWidth((String) reading)+8,14);
  display.setColor(WHITE);
  display.drawRect(4,1,display.getStringWidth((String) reading)+8,14);
  display.drawString(8, 2, (String) reading);
  display.display();
}

void draw_logo(int offset_x, int offset_y, int delay_draw) {
  int logo_l1_size, logo_l2_size;
  display.setColor(BLACK);
  display.fillRect(0,0,(float)OLED_WIDTH,(float)OLED_HEIGHT);
  display.setColor(WHITE);
  //display.clear();

  logo_l2_size = sizeof(logoyubox[0])/(2*sizeof(int));
  logo_l1_size = sizeof(logoyubox)/logo_l2_size/(2*sizeof(int));

  for (int i=0; i<(logo_l1_size); i++) { //
    for (int j=0; j<(logo_l2_size-1); j++) { // 
      if (logoyubox[i][j][0]!=0 && logoyubox[i][j+1][0]!=0) {
        /* Serial.print(logoyubox[i][j][0]);
        Serial.print(',');
        Serial.print(logoyubox[i][j][1]);
        Serial.print('|');
        Serial.print(logoyubox[i][j+1][0]);
        Serial.print(',');
        Serial.println(logoyubox[i][j+1][1]);
        */
        display.drawLine((int)logoyubox[i][j][0]+offset_x, (int)logoyubox[i][j][1]+offset_y, (int)logoyubox[i][j+1][0]+offset_x, (int)logoyubox[i][j+1][1]+offset_y);
        display.display();
        delay(delay_draw);
      }
    }
  }
}


// Adapted from Adafruit_SSD1306
void drawLineShow() {
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(0, 0, i, display.getHeight()-1);
    display.display();
    delay(10);
  }
  for (int16_t i=0; i<display.getHeight(); i+=4) {
    display.drawLine(0, 0, display.getWidth()-1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(0, display.getHeight()-1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
    display.drawLine(0, display.getHeight()-1, display.getWidth()-1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i=display.getWidth()-1; i>=0; i-=4) {
    display.drawLine(display.getWidth()-1, display.getHeight()-1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
    display.drawLine(display.getWidth()-1, display.getHeight()-1, 0, i);
    display.display();
    delay(10);
  }
  delay(250);
  display.clear();
  for (int16_t i=0; i<display.getHeight(); i+=4) {
    display.drawLine(display.getWidth()-1, 0, 0, i);
    display.display();
    delay(10);
  }
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(display.getWidth()-1, 0, i, display.getHeight()-1);
    display.display();
    delay(10);
  }
  delay(250);
}

float getMean(float * val, int arrayCount) {
  long total = 0;
  for (int i = 0; i < arrayCount; i++) {
    total = total + val[i];
  }
  float avg = total/(float)arrayCount;
  return avg;
}

/*
 * Get the standard deviation from an array of ints
 */
float getStdDev(float * val, int arrayCount) {
  float avg = getMean(val, arrayCount);
  long total = 0;
  for (int i = 0; i < arrayCount; i++) {
    total = total + (val[i] - avg) * (val[i] - avg);
  }

  float variance = total/(float)arrayCount;
  float stdDev = sqrt(variance);
  return stdDev/avg;
}
