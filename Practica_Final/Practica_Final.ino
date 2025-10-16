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
  Serial.println("✅ Servo inicializado en posición: " + String(servoPosition));
 
  Serial.print("📡 Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("✅ WiFi connected.");
  Serial.println("📱 IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("🚀 Server started!");
}

void loop(){
  WiFiClient client = server.available();   

  if (client) {                             
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("🆕 New Client.");          
    String currentLine = "";                
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  
      currentTime = millis();
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);  // Esto muestra la solicitud HTTP completa
        header += c;
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            // DEBUG: Mostrar header completo
            Serial.println("📨 Header received:");
            Serial.println(header);
            
            // Procesar la solicitud del slider
            if (header.indexOf("GET /servo?value=") >= 0) {
              int startIndex = header.indexOf("value=") + 6;
              int endIndex = header.indexOf(" ", startIndex);
              if (endIndex == -1) endIndex = header.indexOf("HTTP", startIndex);
              String valueStr = header.substring(startIndex, endIndex);
              Serial.println("🔍 Extracted value string: " + valueStr);
              
              servoPosition = valueStr.toInt();
              
              // Limitar el rango a 0-180 grados
              servoPosition = constrain(servoPosition, 0, 180);
              
              // Mover el servo solo si la posición cambió
              if (servoPosition != previousServoPosition) {
                myservo.write(servoPosition);
                Serial.println("🎯 Servo movido a: " + String(servoPosition) + "°");
                previousServoPosition = servoPosition;
              } else {
                Serial.println("⚡ Posición sin cambios: " + String(servoPosition) + "°");
              }
            } else {
              Serial.println("❌ No se encontró solicitud de servo en el header");
            }
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Página HTML mejorada
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta charset=\"UTF-8\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>");
            client.println("html { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 20px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh;}");
            client.println("body { display: flex; justify-content: center; align-items: center; }");
            client.println(".container { background: white; padding: 40px; border-radius: 20px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); text-align: center; max-width: 600px; width: 100%;}");
            client.println(".slider { width: 100%; height: 20px; margin: 25px 0; background: #e0e0e0; border-radius: 10px; outline: none; opacity: 0.7; transition: opacity 0.2s;}");
            client.println(".slider:hover { opacity: 1; }");
            client.println(".slider::-webkit-slider-thumb { appearance: none; width: 35px; height: 35px; border-radius: 50%; background: #4CAF50; cursor: pointer; border: 3px solid white; box-shadow: 0 2px 5px rgba(0,0,0,0.2);}");
            client.println(".value-display { font-size: 3em; font-weight: bold; color: #2c3e50; margin: 20px 0; text-shadow: 2px 2px 4px rgba(0,0,0,0.1);}");
            client.println(".servo-visual { position: relative; width: 200px; height: 200px; margin: 20px auto;}");
            client.println(".servo-base { width: 120px; height: 60px; background: #555; border-radius: 10px; position: absolute; bottom: 0; left: 50%; transform: translateX(-50%);}");
            client.println(".servo-arm { width: 80px; height: 8px; background: #333; position: absolute; bottom: 30px; left: 60px; transform-origin: left center; border-radius: 4px; transition: transform 0.5s ease;}");
            client.println(".servo-center { width: 20px; height: 20px; background: #ff9800; border-radius: 50%; position: absolute; bottom: 25px; left: 50%; transform: translateX(-50%);}");
            client.println(".title { color: #2c3e50; margin-bottom: 10px; font-size: 2.2em;}");
            client.println(".scale { display: flex; justify-content: space-between; margin-top: 10px; color: #7f8c8d; font-weight: bold;}");
            client.println(".btn-group { display: flex; justify-content: center; gap: 10px; margin: 20px 0; flex-wrap: wrap;}");
            client.println(".btn { padding: 12px 20px; border: none; border-radius: 8px; background: #3498db; color: white; font-size: 16px; cursor: pointer; transition: all 0.3s ease; box-shadow: 0 4px 6px rgba(0,0,0,0.1);}");
            client.println(".btn:hover { transform: translateY(-2px); box-shadow: 0 6px 8px rgba(0,0,0,0.15);}");
            client.println(".btn:active { transform: translateY(0);}");
            client.println(".btn-min { background: #e74c3c; }");
            client.println(".btn-mid { background: #f39c12; }");
            client.println(".btn-max { background: #2ecc71; }");
            client.println(".btn-sweep { background: #9b59b6; }");
            client.println(".input-group { margin: 20px 0; }");
            client.println(".number-input { padding: 12px; font-size: 18px; width: 100px; text-align: center; border: 2px solid #3498db; border-radius: 8px; margin-right: 10px;}");
            client.println(".input-btn { padding: 12px 20px; background: #3498db; color: white; border: none; border-radius: 8px; cursor: pointer; font-size: 16px;}");
            client.println("</style>");
            client.println("<script>");
            client.println("function updateServo(position) {");
            client.println("  position = parseInt(position);");
            client.println("  if(position < 0) position = 0;");
            client.println("  if(position > 180) position = 180;");
            client.println("  ");
            client.println("  document.getElementById('positionValue').innerText = position;");
            client.println("  document.getElementById('servoSlider').value = position;");
            client.println("  document.getElementById('numberInput').value = position;");
            client.println("  updateServoVisual(position);");
            client.println("  ");
            client.println("  var xhr = new XMLHttpRequest();");
            client.println("  xhr.open('GET', '/servo?value=' + position, true);");
            client.println("  xhr.send();");
            client.println("  console.log('Enviando posición: ' + position);");
            client.println("}");
            client.println("");
            client.println("function updateServoVisual(position) {");
            client.println("  var angle = (position - 90) * 1.8; // Convertir a grados de rotación visual");
            client.println("  document.getElementById('servoArm').style.transform = 'rotate(' + angle + 'deg)';");
            client.println("}");
            client.println("");
            client.println("function moveToPosition(position) {");
            client.println("  updateServo(position);");
            client.println("}");
            client.println("");
            client.println("function sweepServo() {");
            client.println("  var positions = [0, 30, 60, 90, 120, 150, 180, 150, 120, 90, 60, 30, 0];");
            client.println("  var delay = 0;");
            client.println("  positions.forEach(function(pos, index) {");
            client.println("    setTimeout(function() { moveToPosition(pos); }, delay);");
            client.println("    delay += 300;");
            client.println("  });");
            client.println("}");
            client.println("");
            client.println("function setCustomPosition() {");
            client.println("  var input = document.getElementById('numberInput');");
            client.println("  var position = parseInt(input.value);");
            client.println("  if(!isNaN(position)) {");
            client.println("    updateServo(position);");
            client.println("  } else {");
            client.println("    alert('Por favor ingresa un número válido entre 0 y 180');");
            client.println("  }");
            client.println("}");
            client.println("");
            client.println("// Permitir Enter en el input");
            client.println("document.getElementById('numberInput').addEventListener('keypress', function(e) {");
            client.println("  if(e.key === 'Enter') {");
            client.println("    setCustomPosition();");
            client.println("  }");
            client.println("});");
            client.println("");
            client.println("// Inicializar visualización");
            client.println("document.addEventListener('DOMContentLoaded', function() {");
            client.println("  updateServoVisual(" + String(servoPosition) + ");");
            client.println("});");
            client.println("</script>");
            client.println("</head>");
            
            client.println("<body>");
            client.println("<div class=\"container\">");
            client.println("<h1 class=\"title\">⚙️ Control de Servo Avanzado</h1>");
            
            // Display de posición
            client.println("<div class=\"value-display\"><span id=\"positionValue\">" + String(servoPosition) + "</span>°</div>");
            
            // Entrada numérica directa
            client.println("<div class=\"input-group\">");
            client.println("<h3>Entrada Directa</h3>");
            client.println("<input type=\"number\" min=\"0\" max=\"180\" value=\"" + String(servoPosition) + "\" class=\"number-input\" id=\"numberInput\" placeholder=\"0-180\">");
            client.println("<button onclick=\"setCustomPosition()\" class=\"input-btn\">Mover</button>");
            client.println("<p style=\"font-size: 12px; color: #666; margin-top: 5px;\">Presiona Enter para enviar</p>");
            client.println("</div>");
            
            // Slider principal
            client.println("<div class=\"slider-container\">");
            client.println("<input type=\"range\" min=\"0\" max=\"180\" value=\"" + String(servoPosition) + "\" class=\"slider\" id=\"servoSlider\" oninput=\"updateServo(this.value)\">");
            client.println("<div class=\"scale\">");
            client.println("<span>0°</span><span>45°</span><span>90°</span><span>135°</span><span>180°</span>");
            client.println("</div>");
            client.println("</div>");
            
            // Botones de posiciones predefinidas
            client.println("<div class=\"btn-group\">");
            client.println("<button onclick=\"moveToPosition(0)\" class=\"btn btn-min\">0° Mínimo</button>");
            client.println("<button onclick=\"moveToPosition(45)\" class=\"btn\">45°</button>");
            client.println("<button onclick=\"moveToPosition(90)\" class=\"btn btn-mid\">90° Centro</button>");
            client.println("<button onclick=\"moveToPosition(135)\" class=\"btn\">135°</button>");
            client.println("<button onclick=\"moveToPosition(180)\" class=\"btn btn-max\">180° Máximo</button>");
            client.println("<button onclick=\"sweepServo()\" class=\"btn btn-sweep\">🔄 Barrido</button>");
            client.println("</div>");
            
            client.println("<p style=\"color: #7f8c8d; margin-top: 25px; font-style: italic;\">Control completo del servomotor con múltiples métodos de entrada</p>");
            client.println("</div>");
            client.println("</body></html>");
            
            client.println();
            
            // DEBUG: Confirmar que se envió la respuesta
            Serial.println("✅ Response sent to client");
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
    Serial.println("🔌 Client disconnected.");
    Serial.println("");
  }
  
  // Pequeño delay para evitar sobrecarga
  delay(10);
}