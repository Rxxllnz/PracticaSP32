

//#include <driver/esp_sleep.h>
//#include <driver/touch.h>

int counter = 0;

// Define el pin touch para despertar
const int TOUCH_PIN1 = 4; // Por ejemplo, GPIO 15
const int TOUCH_PIN2 = 13; // Por ejemplo, GPIO 15
#define THRESHOLD 5000

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  // Configura el pin touch para despertar el ESP32
  esp_sleep_enable_touchpad_wakeup();
  touchAttachInterrupt(TOUCH_PIN1, touchInterrupt, THRESHOLD); 
  touchAttachInterrupt(TOUCH_PIN2, deepsleep, THRESHOLD); 

}

void loop() {
  // Este código no se ejecutará mientras el ESP32 esté en deep sleep

    Serial.println(counter);
  counter ++;
}

// Función que se ejecuta cuando la interrupción táctil se activa
void touchInterrupt() {
  Serial.println("¡Se detectó un toque en el pin táctil!");
}

void deepsleep(){

  Serial.println("ESP32 entrando en modo Deep Sleep. Toca el pin " + String(TOUCH_PIN1) + " para despertar.");
  delay(100);

  // Inicia el Deep Sleep
  esp_deep_sleep_start();
}


