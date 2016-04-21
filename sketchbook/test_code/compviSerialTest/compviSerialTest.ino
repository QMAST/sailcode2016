String myString;
int requestStatus = 0;
unsigned long previousMillis = 0;
int serialDelay = 1000;
int rightLED = 3;
int leftLED = 4;

void setup() {
  pinMode(rightLED, OUTPUT);
  pinMode(leftLED, OUTPUT);
  Serial.begin(57600);
  Serial.println("Starting up...");
}

void loop() {
  unsigned long currentMillis = millis();
  if (requestStatus == 0){
    Serial.println("R");
    requestStatus = 1;
    previousMillis = currentMillis;
  }
  if (Serial.available() > 0) {
    //shrequestStatus = 0;
    myString = Serial.readStringUntil('\n');
    if (myString != ""){
      Serial.println(myString);
    }
    if (myString == "N"){
      digitalWrite(rightLED, LOW);
      digitalWrite(leftLED, LOW);      
    }
    else if (myString.toInt() > 0){
      digitalWrite(rightLED, HIGH);
      digitalWrite(leftLED, LOW);
    }
    else{
      digitalWrite(rightLED, LOW);
      digitalWrite(leftLED, HIGH);
    }
  }
  if (currentMillis - previousMillis >= serialDelay){
    requestStatus = 0;
  }
}
