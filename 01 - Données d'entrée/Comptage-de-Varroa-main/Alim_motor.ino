#include <AccelStepper.h>

// Définir les connexions pour le TMC2226 (broches de l'ESP32)
#define dirPin 19
#define stepPin 18

AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

void setup() {
  // Configurer la vitesse maximale et l'accélération
  stepper.setMaxSpeed(1000);  // Vitesse maximale du moteur
  stepper.setAcceleration(500);  // Accélération du moteur
}

void loop() {
  // Déplacer le moteur de 200 pas dans une direction
  stepper.moveTo(200);
  while (stepper.distanceToGo() != 0) {
    stepper.run();  // Avancer vers la position cible
  }
  delay(1000);

  // Déplacer le moteur de 200 pas dans l'autre direction
  stepper.moveTo(-200);
  while (stepper.distanceToGo() != 0) {
    stepper.run();  // Avancer vers la position cible
  }
  delay(1000);
}