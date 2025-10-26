#define dirPin 12    // nouvelle broche DIR
#define stepPin 13   // nouvelle broche STEP
#define enPin 21     // pin EN, forcée à GND

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);
  
  digitalWrite(enPin, LOW); // activer le driver en le mettant à GND
}

void loop() {
  // tourner dans une direction
  digitalWrite(dirPin, HIGH);
  for (int i = 0; i < 200; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
  delay(1000);

  // tourner dans l'autre direction
  digitalWrite(dirPin, LOW);
  for (int i = 0; i < 200; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
  delay(1000);
}
