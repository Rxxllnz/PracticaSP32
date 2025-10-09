
int ultimo_instante = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);


}

void loop() {
  // put your main code here, to run repeatedly:
  if ((millis() - ultimo_instante) > 300){

    Serial.println("Hola mundo");
    ultimo_instante = millis();
  }

}
