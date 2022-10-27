#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Adafruit_MLX90640.h>

#if CONFIG_IDF_TARGET_ESP32
// Pines a usar para I2C
#define YUBOX_I2C_SDA               SDA
#define YUBOX_I2C_SCL               SCL

#elif CONFIG_IDF_TARGET_ESP32S2
// Pines a usar para I2C
#define YUBOX_I2C_SDA               GPIO_NUM_40
#define YUBOX_I2C_SCL               GPIO_NUM_41

#else
#error Pines de control no definidos para board objetivo!
#endif

Adafruit_MLX90640 mlx;
float frame[32*24]; // buffer for full frame of temperatures

const char* ssid = "WIFI_NOMBRE";
const char* password = "WIFI_PASS";

WebServer server(80);

void handleRoot() {
  /*
  server.send(200, "text/plain", "hello from esp8266!");
  */
  const char * p = "<!DOCTYPE html><html lang=\"en\"><head><title>Camara Termica</title><script type=\"text/javascript\">\n\nfunction f(s) { var i = document.body.getElementsByTagName('img')[0]; i.src = s; } var t = setInterval(function(){f('temp.bmp');}, 100);</script></head><body><img width=\"640\" height=\"480\" src=\"temp.bmp\"/></body></html>";
  server.send(200, "text/html", p);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleTemp();

void setup(void) {
  Serial.begin(115200);

  Wire.setPins(YUBOX_I2C_SDA, YUBOX_I2C_SCL);
  Wire.begin();

  delay(5000);
  Serial.println("Tarjeta funciona luego de esperar 5 segundos para puerto serial en host...");  
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("yubox-camara")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/temp.txt", handleTemp);
  server.on("/temp.bmp", handleTemp2);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  /*************************************************************/

  Serial.println("Adafruit MLX90640 Simple Test");
  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
    while (1) {
      delay(1000);
      Serial.println("MLX90640 not found! Check connections and reboot!");
    }
  }
  Serial.println("Found Adafruit MLX90640");

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);
  
  //mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setMode(MLX90640_CHESS);
  Serial.print("Current mode: ");
  if (mlx.getMode() == MLX90640_CHESS) {
    Serial.println("Chess");
  } else {
    Serial.println("Interleave");    
  }

  mlx.setResolution(MLX90640_ADC_18BIT);
  Serial.print("Current resolution: ");
  mlx90640_resolution_t res = mlx.getResolution();
  switch (res) {
    case MLX90640_ADC_16BIT: Serial.println("16 bit"); break;
    case MLX90640_ADC_17BIT: Serial.println("17 bit"); break;
    case MLX90640_ADC_18BIT: Serial.println("18 bit"); break;
    case MLX90640_ADC_19BIT: Serial.println("19 bit"); break;
  }

  //mlx.setRefreshRate(MLX90640_2_HZ);
  mlx.setRefreshRate(MLX90640_4_HZ);
  Serial.print("Current frame rate: ");
  mlx90640_refreshrate_t rate = mlx.getRefreshRate();
  switch (rate) {
    case MLX90640_0_5_HZ: Serial.println("0.5 Hz"); break;
    case MLX90640_1_HZ: Serial.println("1 Hz"); break; 
    case MLX90640_2_HZ: Serial.println("2 Hz"); break;
    case MLX90640_4_HZ: Serial.println("4 Hz"); break;
    case MLX90640_8_HZ: Serial.println("8 Hz"); break;
    case MLX90640_16_HZ: Serial.println("16 Hz"); break;
    case MLX90640_32_HZ: Serial.println("32 Hz"); break;
    case MLX90640_64_HZ: Serial.println("64 Hz"); break;
  }
  
}

void loop(void) {
  server.handleClient();
}

void handleTemp()
{
  String m = "";

  if (mlx.getFrame(frame) != 0) {
    m += "Failed\n";
  } else {
    for (uint8_t h=0; h<24; h++) {
      for (uint8_t w=0; w<32; w++) {
        float t = frame[h*32 + w];
        char c = '&';
        if (t < 20) c = ' ';
        else if (t < 23) c = '.';
        else if (t < 25) c = '-';
        else if (t < 27) c = '*';
        else if (t < 29) c = '+';
        else if (t < 31) c = 'x';
        else if (t < 33) c = '%';
        else if (t < 35) c = '#';
        else if (t < 37) c = 'X';
        m += c;
      }
      m += "\n";
    }
    
  
  }

  server.send(200, "text/plain", m);
}

const char * bmpheader = "\x42\x4d\x36\x09\x00\x00\x00\x00\x00\x00\x36\x00\x00\x00\x28\x00\x00\x00\x20\x00\x00\x00\x18\x00\x00\x00\x01\x00\x18\x00\x00\x00\x00\x00\x00\x09\x00\x00\x23\x2e\x00\x00\x23\x2e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
void mapPalette(float t, uint8_t *pix);
void handleTemp2()
{
  if (mlx.getFrame(frame) != 0) {
    server.send(500, "text/plain", "Failed\n");
    return;
  }

  uint8_t *data = (uint8_t *)malloc(2358);
  memcpy(data, bmpheader, 54);

  uint8_t *rgb;

  for (int h=0; h< 24; h++) {
    for (int w=0; w<32; w++) {
      float t = frame[h*32 + w];
  
      rgb = data + 54 + (23 - h) * 32*3 + w*3;
/*

      // Color azul
      c = (25.0 - t) * (255.0 / 25.0); if (c > 255) c = 255; if (c < 0) c = 0;
      rgb[0] = (uint8_t)c;

      // Color verde, más allá del rojo
      c = (t - 20.0) * (255.0 / 25.0); if (c > 255) c = 255; if (c < 0) c = 0;
      rgb[1] = (uint8_t)c;

      // Color rojo
      c = t * (255.0 / 25.0); if (c > 255) c = 255; if (c < 0) c = 0;
      rgb[2] = (uint8_t)c;
*/
      mapPalette(t, rgb);    
    }
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, max-age=0, must-revalidate");
  server.send_P(200, "image/bmp", (const char *)data, 2358);
  free(data);
}

#define NUM_PALETTE 120
const uint8_t palette[NUM_PALETTE][3] =
{
    {0,0,0,},
    {0,0,36,},
    {0,0,51,},
    {0,0,66,},
    {0,0,81,},
    {2,0,90,},
    {4,0,99,},
    {7,0,106,},
    {11,0,115,},
    {14,0,119,},
    {20,0,123,},
    {27,0,128,},
    {33,0,133,},
    {41,0,137,},
    {48,0,140,},
    {55,0,143,},
    {61,0,146,},
    {66,0,149,},
    {72,0,150,},
    {78,0,151,},
    {84,0,152,},
    {91,0,153,},
    {97,0,155,},
    {104,0,155,},
    {110,0,156,},
    {115,0,157,},
    {122,0,157,},
    {128,0,157,},
    {134,0,157,},
    {139,0,157,},
    {146,0,156,},
    {152,0,155,},
    {157,0,155,},
    {162,0,155,},
    {167,0,154,},
    {171,0,153,},
    {175,1,152,},
    {178,1,151,},
    {182,2,149,},
    {185,4,149,},
    {188,5,147,},
    {191,6,146,},
    {193,8,144,},
    {195,11,142,},
    {198,13,139,},
    {201,17,135,},
    {203,20,132,},
    {206,23,127,},
    {208,26,121,},
    {210,29,116,},
    {212,33,111,},
    {214,37,103,},
    {217,41,97,},
    {219,46,89,},
    {221,49,78,},
    {223,53,66,},
    {224,56,54,},
    {226,60,42,},
    {228,64,30,},
    {229,68,25,},
    {231,72,20,},
    {232,76,16,},
    {234,78,12,},
    {235,82,10,},
    {236,86,8,},
    {237,90,7,},
    {238,93,5,},
    {239,96,4,},
    {240,100,3,},
    {241,103,3,},
    {241,106,2,},
    {242,109,1,},
    {243,113,1,},
    {244,116,0,},
    {244,120,0,},
    {245,125,0,},
    {246,129,0,},
    {247,133,0,},
    {248,136,0,},
    {248,139,0,},
    {249,142,0,},
    {249,145,0,},
    {250,149,0,},
    {251,154,0,},
    {252,159,0,},
    {253,163,0,},
    {253,168,0,},
    {253,172,0,},
    {254,176,0,},
    {254,179,0,},
    {254,184,0,},
    {254,187,0,},
    {254,191,0,},
    {254,195,0,},
    {254,199,0,},
    {254,202,1,},
    {254,205,2,},
    {254,208,5,},
    {254,212,9,},
    {254,216,12,},
    {255,219,15,},
    {255,221,23,},
    {255,224,32,},
    {255,227,39,},
    {255,229,50,},
    {255,232,63,},
    {255,235,75,},
    {255,238,88,},
    {255,239,102,},
    {255,241,116,},
    {255,242,134,},
    {255,244,149,},
    {255,245,164,},
    {255,247,179,},
    {255,248,192,},
    {255,249,203,},
    {255,251,216,},
    {255,253,228,},
    {255,254,239,},
    {255,255,255,},
};

#define MIN_TEMP_C  10.0f
#define MAX_TEMP_C  50.0f
void mapPalette(float t, uint8_t *pix)
{
  pix[0] = pix[1] = pix[2] = 0;

  // Rango de temperaturas a mapear --> 0.0..1.0
  if (t < MIN_TEMP_C) t = MIN_TEMP_C;
  if (t > MAX_TEMP_C) t = MAX_TEMP_C;
  t = (t - MIN_TEMP_C) / (MAX_TEMP_C - MIN_TEMP_C);

  uint32_t ti;
  float ta, tb;

  ti = (uint32_t)(t * (NUM_PALETTE - 1));
  ta = (float)(ti) / (NUM_PALETTE - 1);
  tb = (float)(ti + 1) / (NUM_PALETTE - 1);

  pix[0] = palette[ti][2];
  pix[1] = palette[ti][1];
  pix[2] = palette[ti][0];
  if (ti < NUM_PALETTE - 1) {
     float r = (t - ta) / (tb - ta);
     pix[0] += r * (palette[ti + 1][2] - palette[ti][2]);
     pix[1] += r * (palette[ti + 1][1] - palette[ti][1]);
     pix[2] += r * (palette[ti + 1][0] - palette[ti][0]);
  }
}
