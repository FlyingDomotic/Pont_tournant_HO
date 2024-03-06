#!/usr/bin/python3
#
#   Test program for rotation Sensor
#
#   Written by Flying Domotic in February 2024

import time
from rotationSensor import RotationSensor
from constants import Constants

# Stepper constants
constants = Constants()

# Declare rotation sensor
angleSensor = RotationSensor(constants.traceDebug)

# Initialize rotation sensor
if constants.traceDebug:
    print("Initializing rotation sensor")
if angleSensor.begin(constants.offsetAngle):
    print("Error detected in begin, exiting...")
    exit(1)

while True:
    angle = angleSensor.getAngle()
    time.sleep(1)