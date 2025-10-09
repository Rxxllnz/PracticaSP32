void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  // read hall effect sensor value
  int val = hallRead();
  
  // print the results to the serial monitor
  Serial.println(val); 
  delay(1000);
}