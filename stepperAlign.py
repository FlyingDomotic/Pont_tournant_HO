#!/usr/bin/python3
#
#   Stepper test program
#
#       Written by Flying Domotic in February 2024

defaultIncrement = 1        # Default increment when using '+' or '-' alone

from stepperCommand import StepperCommand
from rotationSensor import RotationSensor
from constants import Constants

# Stepper constants
constants = Constants()

# Stepper object
stepper = StepperCommand(constants.pulseGpio, constants.directionGpio, constants.enableGpio, constants.degreesPerStep, constants.microStepsPerStep, constants.driverMinimalMicroSec, constants.traceDebug)

# Declare rotation sensor
angleSensor = RotationSensor(constants.traceDebug)

# Stepper initialization
if constants.traceDebug:
    print("Initializing stepper")
stepper.begin()

# Set step duration giving required RPM
stepper.setRPM(constants.requiredRPM)

# Initialize rotation sensor
if constants.traceDebug:
    print("Initializing rotation sensor")
angleSensor.begin(constants.offsetAngle)

currentAngle = 0.0
while True:
    command = input(F"Position (1-{len(constants.indexes)}) or '+/- angle' or 'exit': ").lower()
    deltaAngle = 0.0
    requiredAngle = 0.0
    if command == "end" or command == "exit":
        break
    elif command=='-':
        deltaAngle = - defaultIncrement
        requiredAngle = currentAngle + deltaAngle
    elif command=='+':
        deltaAngle = defaultIncrement
        requiredAngle = currentAngle + deltaAngle
    elif command[0]=='-' or command[0]=='+':
        try:
            deltaAngle = float(command[1:])
        except Exception as e:
            print(f"*** Error parsing {command} as float - {type(e).__name__} at line {e.__traceback__.tb_lineno} of {__file__}: {e} ***")
        requiredAngle = currentAngle + deltaAngle
    else:
        try:
            index = int(command)-1
            if index >=0 and index < len(constants.indexes):
                requiredAngle = constants.indexes[index]
                deltaAngle = requiredAngle - currentAngle
            else:
                print(F"Value outside limit, please specify value between 1 and {len(constants.indexes)}")
        except Exception as e:
            print(f"*** Error parsing {command} as integer - {type(e).__name__} at line {e.__traceback__.tb_lineno} of {__file__}: {e} ***")
    if deltaAngle > 180:
        deltaAngle -= 360
    if deltaAngle < -180:
        deltaAngle += 360
    if constants.traceDebug:
        print(F"Current angle = {currentAngle}, required angle = {requiredAngle}, delta angle = {deltaAngle}")
    stepper.rotateAngle(deltaAngle)
    currentAngle = requiredAngle
    print(F"Read angle: {angleSensor.getAngle()}°")
