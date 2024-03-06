import asyncio
import websockets
import math
from constants import Constants
from rotationSensor import RotationSensor
from stepperCommand import StepperCommand

# Define a callback function to handle incoming WebSocket messages
async def handle_websocket(websocket, path):
    currentAngle = 0
    try:
        while True:
            message = await websocket.recv()
            print(f"Received message: {message}")
            content = message.split('/')
            closestIndex = None
            closestDistance = 9999
            if len(content) == 2:
                x = float(content[0])
                y = float(content[1])
                for i in range(0,len(constants.xPos)):
                    if abs(x - constants.xPos[i]) < constants.maxDelta and abs(y - constants.yPos[i]) < constants.maxDelta:
                        distance = math.sqrt((abs(x - constants.xPos[i]) ** 2) + (abs(y - constants.yPos[i]) ** 2))
                        if distance < closestDistance:
                            closestDistance = distance
                            closestIndex = i
            ###currentAngle = angleSensor.getAngle()
            if closestIndex != None:
                requiredAngle = constants.indexes[closestIndex]
                deltaAngle = requiredAngle - currentAngle
                if deltaAngle > 180:
                    deltaAngle -= 360
                if deltaAngle < -180:
                    deltaAngle += 360
                microStepCount, anglePerMicroStep = stepper.dataForAngle(deltaAngle)
                if constants.traceDebug:
                    print(F"Current angle = {currentAngle}, required angle = {requiredAngle}, delta angle = {deltaAngle}, micro step count = {microStepCount}, angle per micro step = {anglePerMicroStep}")
                for i in range(microStepCount):
                    stepper.turnOneMicroStep()
                    currentAngle += anglePerMicroStep
                    response = f"{currentAngle}"
                    await websocket.send(response)
                currentAngle = requiredAngle
                print(F"Angle after move: {angleSensor.getAngle()}°")            
                response = f"{requiredAngle}"
                await websocket.send(response)
            else:
                response = f"{currentAngle}"
                await websocket.send(response)


    except websockets.ConnectionClosed:
        pass

if __name__ == "__main__":
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

    # Start the WebSocket server
    start_server = websockets.serve(handle_websocket, "0.0.0.0", 8001, max_queue=1, write_limit=1)

    asyncio.get_event_loop().run_until_complete(start_server)
    asyncio.get_event_loop().run_forever()