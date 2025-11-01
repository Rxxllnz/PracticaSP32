#include <WiFi.h>
#include <ESP32Servo.h>
#include <EEPROM.h>

#define EEPROM_SIZE 1  // Solo 1 byte

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
unsigned long previousTime2 = millis();
const long timeoutTime = 2000;

const int botonEEProm = 2;
const int botonSleep = 0;

// Variables para debounce del botón Sleep
volatile bool sleepRequested = false;
int sleepButtonState = HIGH;
int lastSleepButtonState = HIGH;
unsigned long lastSleepDebounceTime = 0;
const unsigned long sleepDebounceDelay = 50;

void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  
  // Configurar el servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // Frecuencia estándar para servos
  myservo.attach(servoPin, 500, 2400); // Pin, pulso mínimo, pulso máximo
  
  // Posición inicial del servo

  EEPromLectura();

  if (180 >= servoPosition && servoPosition >= 0){

    myservo.write(servoPosition);
  }else{
    myservo.write(90);
  }
    myservo.write(servoPosition);
  Serial.println("Servo inicializado en posición: " + String(servoPosition));
 
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Servidor iniciado");

  // Configurar interrupción para el botón EEprom
  pinMode(botonEEProm, INPUT_PULLUP);
  pinMode(botonSleep, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(botonEEProm), EEPromEscritura, FALLING);
  // ELIMINADO: attachInterrupt para botonSleep - ahora se maneja en loop()
}

void loop(){
  // Verificar si se debe entrar en sleep
  checkSleepButton();
  
  WiFiClient client = server.available();   

  if (client) {                             
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("Nuevo cliente");          
    String currentLine = "";                
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  
      currentTime = millis();
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);
        header += c;
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            Serial.println("Header recibido:");
            Serial.println(header);
            
            // Procesar la solicitud del slider
            if (header.indexOf("GET /servo?value=") >= 0) {
              int startIndex = header.indexOf("value=") + 6;
              int endIndex = header.indexOf(" ", startIndex);
              if (endIndex == -1) endIndex = header.indexOf("HTTP", startIndex);
              String valueStr = header.substring(startIndex, endIndex);
              Serial.println("Valor extraido: " + valueStr);
              
              servoPosition = valueStr.toInt();
              
              // Limitar el rango a 0-180 grados
              servoPosition = constrain(servoPosition, 0, 180);
              
              // Mover el servo solo si la posición cambió
              if (servoPosition != previousServoPosition) {
                myservo.write(servoPosition);
                Serial.println("Servo movido a: " + String(servoPosition) + "°");
                previousServoPosition = servoPosition;
              } else {
                Serial.println("Posición sin cambios: " + String(servoPosition) + "°");
              }
            } else {
              Serial.println("No se encontró solicitud de servo en el header");
            }
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            //Creamos la pagina Web
            paginaHTML(client);
            
            Serial.println("Respuesta enviada al cliente");
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
    Serial.println("Cliente desconectado");
    Serial.println("");
  }
  
  delay(10);
}

void checkSleepButton() {
  int reading = digitalRead(botonSleep);

  // Detectar cambio de estado
  if (reading != lastSleepButtonState) {
    lastSleepDebounceTime = millis();
  }

  // Esperar debounce y confirmar estado
  if ((millis() - lastSleepDebounceTime) > sleepDebounceDelay) {
    if (reading != sleepButtonState) {
      sleepButtonState = reading;

      // Solo actuar cuando se presiona el botón (LOW)
      if (sleepButtonState == LOW) {
        Serial.println("Botón sleep presionado - Preparando deep sleep...");
        goToSleep();
      }
    }
  }

  lastSleepButtonState = reading;
}

void paginaHTML(WiFiClient client){
  // Página HTML simplificada
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  client.println("<style>");
  client.println("html { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #f5f5f5;}");
  client.println("body { display: flex; justify-content: center; align-items: center; min-height: 100vh;}");
  client.println(".container { background: white; padding: 30px; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); text-align: center; max-width: 500px; width: 100%;}");
  client.println(".slider { width: 100%; height: 25px; margin: 20px 0; background: #e0e0e0; border-radius: 10px; outline: none;}");
  client.println(".slider::-webkit-slider-thumb { appearance: none; width: 30px; height: 30px; border-radius: 50%; background: #4CAF50; cursor: pointer; border: 3px solid white; box-shadow: 0 2px 5px rgba(0,0,0,0.2);}");
  client.println(".value-display { font-size: 2.5em; font-weight: bold; color: #2c3e50; margin: 15px 0;}");
  client.println(".title { color: #2c3e50; margin-bottom: 10px; font-size: 1.8em;}");
  client.println(".scale { display: flex; justify-content: space-between; margin-top: 10px; color: #7f8c8d; font-weight: bold;}");
  client.println(".btn-group { display: flex; justify-content: center; gap: 8px; margin: 15px 0; flex-wrap: wrap;}");
  client.println(".btn { padding: 10px 15px; border: none; border-radius: 6px; background: #3498db; color: white; font-size: 14px; cursor: pointer; transition: background 0.3s;}");
  client.println(".btn:hover { background: #2980b9; }");
  client.println(".btn-min { background: #e74c3c; }");
  client.println(".btn-mid { background: #f39c12; }");
  client.println(".btn-max { background: #2ecc71; }");
  client.println(".btn-sweep { background: #9b59b6; }");
  client.println(".input-group { margin: 15px 0; }");
  client.println(".number-input { padding: 10px; font-size: 16px; width: 80px; text-align: center; border: 2px solid #3498db; border-radius: 5px; margin-right: 8px;}");
  client.println(".input-btn { padding: 10px 15px; background: #3498db; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 14px;}");
  client.println("</style>");
  client.println("<script>");
  client.println("let sliderTimeout;");
  client.println("let currentPosition = " + String(servoPosition) + ";");
  client.println("");
  client.println("function updateDisplay(position) {");
  client.println("  document.getElementById('positionValue').innerText = position;");
  client.println("  document.getElementById('servoSlider').value = position;");
  client.println("  document.getElementById('numberInput').value = position;");
  client.println("  currentPosition = position;");
  client.println("}");
  client.println("");
  client.println("function sendServoPosition(position) {");
  client.println("  position = parseInt(position);");
  client.println("  if(position < 0) position = 0;");
  client.println("  if(position > 180) position = 180;");
  client.println("  ");
  client.println("  updateDisplay(position);");
  client.println("  ");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.open('GET', '/servo?value=' + position, true);");
  client.println("  xhr.send();");
  client.println("  console.log('Posicion enviada: ' + position + '°');");
  client.println("}");
  client.println("");
  client.println("function onSliderInput(position) {");
  client.println("  updateDisplay(position);");
  client.println("  // Limpiar timeout anterior");
  client.println("  clearTimeout(sliderTimeout);");
  client.println("  // Establecer nuevo timeout para enviar después de 300ms sin movimiento");
  client.println("  sliderTimeout = setTimeout(function() {");
  client.println("    sendServoPosition(position);");
  client.println("  }, 300);");
  client.println("}");
  client.println("");
  client.println("function onSliderChange(position) {");
  client.println("  // Enviar inmediatamente cuando se suelta el slider");
  client.println("  clearTimeout(sliderTimeout);");
  client.println("  sendServoPosition(position);");
  client.println("}");
  client.println("");
  client.println("function moveToPosition(position) {");
  client.println("  sendServoPosition(position);");
  client.println("}");
  client.println("");
  client.println("function sweepServo() {");
  client.println("  var positions = [0, 45, 90, 135, 180, 135, 90, 45, 0];");
  client.println("  var delay = 0;");
  client.println("  positions.forEach(function(pos, index) {");
  client.println("    setTimeout(function() { moveToPosition(pos); }, delay);");
  client.println("    delay += 500;");
  client.println("  });");
  client.println("}");
  client.println("");
  client.println("function setCustomPosition() {");
  client.println("  var input = document.getElementById('numberInput');");
  client.println("  var position = parseInt(input.value);");
  client.println("  if(!isNaN(position)) {");
  client.println("    sendServoPosition(position);");
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
  client.println("</script>");
  client.println("</head>");
  
  client.println("<body>");
  client.println("<div class=\"container\">");
  client.println("<h1 class=\"title\">Control de Servo</h1>");
  
  // Display de posición
  client.println("<div class=\"value-display\"><span id=\"positionValue\">" + String(servoPosition) + "</span>°</div>");
  
  // Entrada numérica directa
  client.println("<div class=\"input-group\">");
  client.println("<input type=\"number\" min=\"0\" max=\"180\" value=\"" + String(servoPosition) + "\" class=\"number-input\" id=\"numberInput\" placeholder=\"0-180\">");
  client.println("<button onclick=\"setCustomPosition()\" class=\"input-btn\">Mover</button>");
  client.println("</div>");
  
  // Slider principal - ahora con onchange
  client.println("<div class=\"slider-container\">");
  client.println("<input type=\"range\" min=\"0\" max=\"180\" value=\"" + String(servoPosition) + "\" class=\"slider\" id=\"servoSlider\" ");
  client.println("       oninput=\"onSliderInput(this.value)\" ");
  client.println("       onchange=\"onSliderChange(this.value)\">");
  client.println("<div class=\"scale\">");
  client.println("<span>0°</span><span>45°</span><span>90°</span><span>135°</span><span>180°</span>");
  client.println("</div>");
  client.println("</div>");
  
  // Botones de posiciones predefinidas
  client.println("<div class=\"btn-group\">");
  client.println("<button onclick=\"moveToPosition(0)\" class=\"btn btn-min\">0°</button>");
  client.println("<button onclick=\"moveToPosition(45)\" class=\"btn\">45°</button>");
  client.println("<button onclick=\"moveToPosition(90)\" class=\"btn btn-mid\">90°</button>");
  client.println("<button onclick=\"moveToPosition(135)\" class=\"btn\">135°</button>");
  client.println("<button onclick=\"moveToPosition(180)\" class=\"btn btn-max\">180°</button>");
  client.println("<button onclick=\"sweepServo()\" class=\"btn btn-sweep\">Barrido</button>");
  client.println("</div>");
  
  client.println("<p style=\"color: #7f8c8d; margin-top: 20px; font-size: 14px;\">El slider envía la posición al soltar el click</p>");
  client.println("</div>");
  client.println("</body></html>");
  
  client.println();
}

void EEPromEscritura(){
  if ((millis() -  previousTime) >=3000){
    EEPROM.write(0, servoPosition);  
    EEPROM.commit();
    Serial.println("Valor guardado");
    previousTime = millis();
  }

}

void EEPromLectura(){
  servoPosition = EEPROM.read(0);  
}

void goToSleep(){
  Serial.println("Preparando deep sleep...");
  
  // Dar tiempo para que suelten el botón
  delay(1000);
  
  // Configurar wake-up por el MISMO botón (GPIO0)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);
  
  Serial.println("Entrando en deep sleep... Presiona el mismo botón para despertar");
  delay(100);
  
  esp_deep_sleep_start();
}