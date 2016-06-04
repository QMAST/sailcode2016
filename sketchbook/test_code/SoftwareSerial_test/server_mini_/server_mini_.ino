void setup() {
  Serial.begin(19200);
}

void loop() {
  if (Serial.available() >= 2) {
    Serial.write(Serial.read()+1); //echo
    Serial.write(Serial.read()+2); //echo
  }
}
