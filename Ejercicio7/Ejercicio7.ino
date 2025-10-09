void setup() 
{
    Serial.begin(115200);
    delay(5000);
}

int counter = 0;

void loop() {
  Serial.println(counter);
  counter ++;

  esp_sleep_enable_timer_wakeup(2 * 1000000); //light sleep durante 2 segundos
  esp_light_sleep_start();  
}