#include <WiFi.h>
#include <ESP32Servo.h>

const char* ssid = "Raulordenador";
const char* password = "SistemasEmbebidos";

WiFiServer server(80);

String header;

// Variables para el servo
Servo myservo;
const int servoPin = 25;  // GPIO25 para el servo
int servoPosition = 90;   // Posición inicial del servo (90°)
int previousServoPosition = 90;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  
  // Configurar el servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // Frecuencia estándar para servos
  myservo.attach(servoPin, 500, 2400); // Pin, pulso mínimo, pulso máximo
  
  // Posición inicial del servo
  myservo.write(servoPosition);
 
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

void loop(){
  WiFiClient client = server.available();   

  if (client) {                             
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          
    String currentLine = "";                
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  
      currentTime = millis();
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);                    
        header += c;
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            // Procesar la solicitud del slider
            if (header.indexOf("GET /servo?value=") >= 0) {
              int startIndex = header.indexOf("value=") + 6;
              int endIndex = header.indexOf(" ", startIndex);
              String valueStr = header.substring(startIndex, endIndex);
              servoPosition = valueStr.toInt();
              
              // Limitar el rango a 0-180 grados
              servoPosition = constrain(servoPosition, 0, 180);
              
              // Mover el servo solo si la posición cambió
              if (servoPosition != previousServoPosition) {
                myservo.write(servoPosition);
                previousServoPosition = servoPosition;
                Serial.print("Servo movido a: ");
                Serial.println(servoPosition);
              }
            }
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Página HTML con el slider
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>");
            client.println("html { font-family: Times New Roman; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".slider-container { margin: 20px; }");
            client.println(".slider { width: 80%; height: 40px; }");
            client.println(".value-display { font-size: 24px; margin: 10px; }");
            client.println("</style>");
            client.println("<script>");
            client.println("function updateServo(position) {");
            client.println("  document.getElementById('positionValue').innerText = position + '°';");
            client.println("  var xhr = new XMLHttpRequest();");
            client.println("  xhr.open('GET', '/servo?value=' + position, true);");
            client.println("  xhr.send();");
            client.println("}");
            client.println("</script>");
            client.println("</head>");
            
            client.println("<body>");
            client.println("<h1>Control de Servo</h1>");
            client.println("<div class=\"slider-container\">");
            client.println("<input type=\"range\" min=\"0\" max=\"180\" value=\"" + String(servoPosition) + "\" class=\"slider\" id=\"servoSlider\" oninput=\"updateServo(this.value)\">");
            client.println("<div class=\"value-display\">Grados: <span id=\"positionValue\">" + String(servoPosition) + "</span></div>");
            client.println("</div>");
            client.println("</body></html>");
            
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;     
        }
      }
    }

    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}