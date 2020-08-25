#include <WiFi.h>

const char* ssid     = "XXXXX";
const char* password = "XXXXX";

int level;
WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    pinMode(4, OUTPUT);      //   pin mode

    delay(10);

    // Inicio de conexion wifi 

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

int value = 0;

void loop(){
 WiFiClient client = server.available();   

  if (client) {                             // get a client
    //Serial.println("New Client.");          
    String currentLine = "";               
    while (client.connected()) {
      
      if (client.available()) {            
        char c = client.read();           
        Serial.write(c);                    
        if (c == '\n') {                    


          if (currentLine.length() == 0) {
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            
            client.print("  YUBOX  MODULE <br><br>");
            client.print("Click <a href=\"/AH\">aqui</a> Encender LED <br>");
            client.print("Click <a href=\"/AL\">aqui</a> Apagar LED <br>");

            client.println();
            break;
          } else {    
            currentLine = "";
          }
        } else if (c != '\r') { 
          currentLine += c;      
        }

        // Verifica las solicitudes del cliente 
        if (currentLine.endsWith("GET /AH")) {
          digitalWrite(4, HIGH); level=20; digitalWrite(4, HIGH); Serial.println("RELE A on");
    
        }
        if (currentLine.endsWith("GET /AL")) {
          digitalWrite(4, LOW);     digitalWrite(4, LOW); Serial.println("RELE A off");  // GET /AL deactiva rele A
        }


      }
    }
    // cierra sesion 
    client.stop();
    
      }        
}
