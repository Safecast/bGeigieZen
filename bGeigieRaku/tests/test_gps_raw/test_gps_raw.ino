void setup()
{
  Serial.begin(57600);

  // Serial GPS with pins defined on Rob's board
  Serial1.begin(9600, SERIAL_8N1, 33, 32);
}

void loop()
{
  while (Serial1.available())
  {
    Serial.write(Serial1.read());
  }
}
