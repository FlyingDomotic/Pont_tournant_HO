#
#   This code manages a ModBus rotary encoder (QY3806-485RTU)
#       It mainly returns angular position giving an initial offset
#
#       Written by Flying Domotic in February 2024

class RotationSensor:
    # Class initialization 
    #   input:
    #       debug: debug flag (True = debug, False = no trace)
    def __init__(self, debug):
        # Internal variables
        self.offsetAngle = 0.0  # Angle of zero degrees position (depends on hardware)
        self.traceDebug = debug # Trace debug messages?
        self.client = None      # Modbus client

    # Class destructor
    def __del__(self):
        pass

    # Ask modbus sensor for motor angle. Returned value is in degrees, corrected by an offset.
    def getAngle(self):
        if self.traceDebug:
            print("Reading register 0")
            data=256
            print(f"Data: {data}, angle: {self.computeAngle(data)}")
        return self.computeAngle(data)

    # Convert a value read in modbus register 0 to an angle in degrees, giving an initial offset
    #   input:
    #       data: device data to convert to angle
    #   returns:
    #       angle, in degrees, corresponding to data
    def computeAngle(self, data):
        return round((360 * data / 65536) - self.offsetAngle, 1) % 360

    # Initialize rotation sensor
    #   input:
    #       offset: zero offset angle in degrees
    #       port: serial port where rotation sensor is connected to
    #       baudrate: serial port speed in bauds
    #   returns:
    #       error flag (False: no error detected, True: an error has been detected (you should probably exit code))
    def begin(self, offset=None, port='/dev/serial0', baudrate=9600):
        if offset != None:
            self.offsetAngle = offset
        if self.traceDebug:
            print(F"Connected to modbus port {port}, speed={baudrate}")
        return False