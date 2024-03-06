#
#   This code manages a stepper motor with at least pulse and direction GPIO.
#       Motor's characteristics should include:
#           degreesPerStep: motor's number of degrees per step
#           microStepsPerStep: drivers number of micro-steps per step
#           driverMinimalMicroSec: drivers minimal signal duration in micro-seconds
#           requiredRPM: required motor RPM
#           
#       Motor is connected to the following GPIO:
#           pulseGpio: driver's pulse GPIO (mandatory)
#           directionGpio: driver's direction GPIO (mandatory)
#           enableGpio: drivers enable GPIO (optional)
#
#       Written by Flying Domotic in February 2024

import time

class StepperCommand:
    # Class initialization 
    #   input:
    #       pulseGpio: GPIO where drivers pulse is connected to
    #       directionGpio: GPIO where drivers direction is connected to
    #       enableGpio: GPIO where drivers enable is connected to (can be zero)
    #       degreesPerStep: stepper number of degrees per step (usually 1.8)
    #       microStepsPerStep: number of microsteps per step (usually 1, 2, 4, 8...)
    #       driverMinimalMicroSec: minimum microseconds time driver needs to acquire a signal (around 2-3 uS)
    #       traceDebug: True to trace debug message
    def __init__(self, pulseGpio, directionGpio, enableGpio, degreesPerStep, microStepsPerStep, driverMinimalMicroSec, traceDebug):
        # Internal variables
        self.degreesPerStep = degreesPerStep                # Motor's number of degrees per step
        self.microStepsPerStep = microStepsPerStep          # Drivers number of micro-steps per step
        self.driverMinimalMicroSec = driverMinimalMicroSec  # Drivers minimal signal duration in micro-seconds
        self.requiredRPM = 1                                # Required motor RPM
        self.traceDebug = traceDebug                        # Trace debug messages?
        self.pulseGpio = pulseGpio                          # Driver's pulse GPIO
        self.directionGpio = directionGpio                  # Driver's direction GPIO
        self.enableGpio = enableGpio                        # Drivers enable GPIO

    # Class destructor
    def __del__(self):
        if self.traceDebug:
            print ("Reset motor")
        if self.driverMinimalMicroSec:
            time.sleep(self.driverMinimalMicroSec / 1E6)
        if self.driverMinimalMicroSec:
            time.sleep(self.driverMinimalMicroSec / 1E6)

    #   Set step duration giving expected RPM (rotation per minute)
    #       input:
    #           RPM : expected rotations per minute
    def setRPM(self, RPM):
        # Compute steps needed to make 1 full turn
        self.stepsPerRotation = 360 * self.microStepsPerStep / self.degreesPerStep
        # Compute 1 step duration (in seconds)
        oneStepDuration = 60 / (RPM * self.stepsPerRotation)
        if self.traceDebug:
            print(f"Each step will last {oneStepDuration * 1000} ms to make {RPM} RPM at {self.degreesPerStep}° per step at {self.microStepsPerStep} micro-steps")
        # Check for driver's minimum duration
        if (oneStepDuration / 2) > (self.driverMinimalMicroSec * 1E6):
            print(f"*** Step duration is too low, set to 1 ***")
            # Force 1 step per second (very slow)
            oneStepDuration = 1
        self.stepDuration = oneStepDuration

    #   Moves motor giving specified angle
    #       input:
    #           angle: angle to rotate motor, in degrees
    def rotateAngle(self, angle):
        microSteps = round(angle * self.microStepsPerStep / self.degreesPerStep)
        if self.traceDebug:
            print(f"Motor needs {microSteps} micro-steps of {self.stepDuration}s to turn about {angle}° in {round(microSteps * self.stepDuration)} s")

        # Set correct direction
        time.sleep(self.driverMinimalMicroSec / 1E6)
        
        # Rotate motor the right number of micro-steps
        for i in range(abs(microSteps)):
            time.sleep(self.stepDuration / 2)
            time.sleep(self.stepDuration / 2)
            
    #   Return data needed to move a given angle. Set the right direction.
    #       input:
    #           angle: angle to rotate motor, in degrees
    #       output:
    #           count of micro steps to turn the angle (unsigned)
    #           angle moved by one microStep (signed)
    #       side effect:
    #           stepper direction has been set
    def dataForAngle(self, angle):
        microSteps = round(angle * self.microStepsPerStep / self.degreesPerStep)
        if microSteps >= 0 :
            anglePerMicroStep = self.degreesPerStep / self.microStepsPerStep
        else:
            anglePerMicroStep = -self.degreesPerStep / self.microStepsPerStep
        if self.traceDebug:
            print(f"Motor needs {microSteps} micro-steps of {self.stepDuration}s to turn about {angle}° in {round(microSteps * self.stepDuration)} s")

        # Set correct direction
        time.sleep(self.driverMinimalMicroSec / 1E6)
        
        return abs(microSteps), anglePerMicroStep

    #   Turns stepper one micro step
    def turnOneMicroStep(self):
        time.sleep(self.stepDuration / 2)
        time.sleep(self.stepDuration / 2)

    #   GPIO initialization
    def begin(self):
        pass