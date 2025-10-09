int i = 0;
int ultimo_tiempo = 0;

void setup() {
  Serial.begin(115200);


}

void loop() {

  if((millis() - ultimo_tiempo) > 300){

    float sine_1 = 1 * sin(i * M_PI / 180);
    ultimo_tiempo = millis();
    Serial.println(sine_1);
    i++;

  }

}