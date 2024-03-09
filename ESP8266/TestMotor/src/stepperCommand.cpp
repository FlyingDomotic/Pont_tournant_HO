#include <stepperCommand.h>
#include <Arduino.h>

// Class initialization 
//   input:
//       _pulsePin: Pin where drivers pulse is connected to
//       _directionPin: Pin where drivers direction is connected to
//       _enablePin: Pin where drivers enable is connected to (can be zero)
//       _degreesPerStep: stepper number of degrees per step (usually 1.8)
//       _microStepsPerStep: number of microsteps per step (usually 1, 2, 4, 8...)
//       _driverMinimalMicroSec: minimum microseconds time driver needs to acquire a signal (around 2-3 uS)
//       _traceDebug: True to trace debug message
StepperCommand::StepperCommand(uint8_t _pulsePin, uint8_t _directionPin, uint8_t _enablePin, float _degreesPerStep, uint16_t _microStepsPerStep, uint8_t _driverMinimalMicroSec, bool _traceDebug) {
  // Internal variables
  degreesPerStep = _degreesPerStep;                // Motor's number of degrees per step
  microStepsPerStep = _microStepsPerStep;          // Drivers number of micro-steps per step
  driverMinimalMicroSec = _driverMinimalMicroSec;  // Drivers minimal signal duration in micro-seconds
  requiredRPM = 1;                                 // Required motor RPM
  traceDebug = _traceDebug;                        // Trace debug messages?
  pulsePin = _pulsePin;                            // Driver's pulse Pin
  directionPin = _directionPin;                    // Driver's direction Pin
  enablePin = _enablePin;                          // Driver's enable Pin
}

//  Pin initialization
void StepperCommand::begin(float _RPM){
  if (enablePin != -1) {
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, 0);
    delayMicroseconds(driverMinimalMicroSec);
  }
  digitalWrite(pulsePin, 0);
  pinMode(pulsePin, OUTPUT);
  digitalWrite(directionPin, 0);
  pinMode(directionPin, OUTPUT);
  if (_RPM) {
    setRPM(_RPM);
  }
}

//  Set step duration giving expected RPM (rotation per minute)
//       input:
//           _RPM : expected rotations per minute
void StepperCommand::setRPM(float _RPM){
  // Compute steps needed to make 1 full turn
  float stepsPerRotation = 360.0 * microStepsPerStep / degreesPerStep;
  // Compute 1 step duration (in seconds)
  float oneStepDuration = 60 / (_RPM * stepsPerRotation);
  // Check for driver's minimum duration
  if ((oneStepDuration / 2.0) > (driverMinimalMicroSec * 1E6)) {
      Serial.println("*** Step duration is too low, set to 1 ***");
      // Force 1 step per second (very slow)
      oneStepDuration = 1;
  }
  stepDuration = oneStepDuration;
  if (traceDebug) {
      Serial.printf("Each step will last %.2f ms to make %.1f RPM at %.1f° per step at %d micro-steps\n", oneStepDuration * 1000, _RPM, degreesPerStep, microStepsPerStep);
  }
}

//  Moves motor giving specified angle
//       input:
//           _angle: angle to rotate motor, in degrees
void StepperCommand::rotateAngle(float _angle){
  unsigned long microSteps = abs(_angle) * microStepsPerStep / degreesPerStep;
  if (traceDebug) {
      Serial.printf("Motor needs %ld micro-steps of %.0fms to turn about %.1f° in %.1f s\n", microSteps, stepDuration * 1000.0, _angle, microSteps * stepDuration);
  }

  // Set correct direction
  if (_angle < 0.0) {
    digitalWrite(directionPin, 1);
  } else {
    digitalWrite(directionPin, 0);
  }
  delayMicroseconds(driverMinimalMicroSec);
  
  // Rotate motor the right number of micro-steps
  for (unsigned long i=0; i < microSteps; i++){
    digitalWrite(pulsePin, 1);
    delay(stepDuration * 1E3 / 2.0);
    digitalWrite(pulsePin, 0);
    delay(stepDuration * 1E3 / 2.0);
  }
}

//  Return number of microsteps needed to move a given angle. Set the right direction.
//       input:
//           _angle: angle to rotate motor, in degrees
//       output:
//           count of micro steps to turn the angle
//       side effect:
//           stepper direction has been set
unsigned long StepperCommand::microStepsForAngle(float _angle){
  unsigned long microSteps = abs(_angle) * microStepsPerStep / degreesPerStep;
  if (traceDebug) {
      Serial.printf("Motor needs %ld micro-steps of %.0fms to turn about %f° in %f s\n", microSteps, stepDuration * 1000.0, _angle, microSteps * stepDuration);
  }
  // Set correct direction
  if (_angle < 0.0) {
    digitalWrite(directionPin, 1);
  } else {
    digitalWrite(directionPin, 0);
  }
  delayMicroseconds(driverMinimalMicroSec);
  return microSteps;
}

//  Returns angle corresponding to one micro step
float StepperCommand::anglePerMicroStep(void){
  return degreesPerStep / microStepsPerStep;
}

//  Turns stepper one micro step
void StepperCommand::turnOneMicroStep(void){
  digitalWrite(pulsePin, 1);
  delay(stepDuration * 1E3 / 2.0);
  digitalWrite(pulsePin, 0);
  delay(stepDuration * 1E3 / 2.0);
}
