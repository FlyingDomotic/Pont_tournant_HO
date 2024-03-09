#include <arduino.h>
#include <../../constants.h>
#include <stepperCommand.h>

float degrees = 90.0;
StepperCommand stepper(pulsePin, directionPin, enablePin, degreesPerStep, microStepsPerStep, driverMinimalMicroSec, traceDebug);

void setup() {
  Serial.begin(74880);
  Serial.printf("Rotate motor test: will rotate motor %.1f° in one way and reverse\n",degrees);
  stepper.begin(2.0);
}

void loop() {
  stepper.rotateAngle(degrees);
  delay(1000);
  stepper.rotateAngle(-degrees);
  delay(1000);
}