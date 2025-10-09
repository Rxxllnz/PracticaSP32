
#define TOUCH_0 4

void setup() 
{
  Serial.begin(115200);
}

void loop() {
  const int touchValue = touchRead(TOUCH_0); 
  Serial.println(touchValue);  
  delay(1000);
}