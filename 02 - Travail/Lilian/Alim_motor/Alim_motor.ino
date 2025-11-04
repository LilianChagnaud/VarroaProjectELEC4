#include <AccelStepper.h>

#define dirPin 12    // nouvelle broche DIR
#define stepPin 13   // nouvelle broche STEP

AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

void setup() {
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1000);
}

void loop() {
  // tourner dans une direction
  stepper.moveTo(200);
  while(stepper.distanceToGo() != 0){
    stepper.run();
  }
  delay(1000);

  // tourner dans l'autre direction
  stepper.moveTo(-200);
  while (stepper.distanceToGo() != 0) {
    stepper.run();  // Avancer vers la position cible
  }
  delay(1000);
}
