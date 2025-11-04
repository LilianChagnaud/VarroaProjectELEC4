#include <AccelStepper.h>

#define dirPin 12
#define stepPin 13

AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

// === PARAMÈTRES MECANIQUES ===
const float stepsPerMM = 40.0; // 1 tour moteur(1600 pas) équivaut à 40mm
const float offsetMin = 35.0; // Distance entre le bord de la planche et la limite de translation du support de caméra en mm
const float offsetMax = 45.0;
const float cameraSupportLength = 75.0;
const float totalLength = 540.0; // longueur de la planche en mm
const float usableLength = totalLength - offsetMin - offsetMax - cameraSupportLength; //383mm
const float minPos = 0; // Position niveau moteur en mm
const float maxPos = usableLength;

// === PARAMÈTRES MOTEUR ===
//const int stepsPerRevolution = 1600; // 1 tour moteur = 1600 pas (en 1/8 microstep)
const int speed = 1500;              // vitesse max (pas/s)
const int accel = 1000;               // accélération (pas/s²)

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(speed);
  stepper.setAcceleration(accel);
  stepper.setCurrentPosition(0);

  Serial.println("Prêt. Entrez une position entre 0 et 383mm :");
}

void loop() {
  if(Serial.available() > 0){
    float targetMM = Serial.parseFloat();

    while (Serial.available() > 0) Serial.read(); // vide le buffer pour les \n et \r

    if (targetMM < minPos) targetMM = minPos;
    if (targetMM > maxPos) targetMM = maxPos;

    long targetSteps = targetMM * stepsPerMM;
    stepper.moveTo(targetSteps);

    Serial.print("Deplacement vers ");
    Serial.print(targetMM);
    Serial.println(" mm ...");

  }
  stepper.run();
}
