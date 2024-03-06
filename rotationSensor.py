#
#   This code manages a ModBus rotary encoder (QY3806-485RTU)
#       It mainly returns angular position giving an initial offset
#
#       Written by Flying Domotic in February 2024

import pymodbus.exceptions
from pymodbus.client.sync import ModbusSerialClient

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
        if self.client != None:
            self.client.close()

    # Ask modbus sensor for motor angle. Returned value is in degrees, corrected by an offset.
    def getAngle(self):
        if self.traceDebug:
            print("Reading register 0")
        try:
            # Read register 0 (angle)
            rr = self.client.read_holding_registers(0x00,1,unit=1)
        except pymodbus.ModbusException as exc:
            # Library raised an error (an returned nothing)
            print("Error reading register 0")
            print(f"Received ModbusException({exc}) from library")
            return None

        if rr.isError():
            # Library returned an error
            print("Error reading register 0")
            print(f"Received Modbus library error({rr})")
            return None

        if isinstance(rr, pymodbus.ExceptionResponse):
            #Library returned an exception
            print("Error reading register 0")
            print(f"Received Modbus library exception ({rr})")
            return None

        if self.traceDebug:
            print(rr)
            print(rr.registers)
            data=rr.registers[int(0)]
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
            print(F"Connecting to modbus port {port}, speed={baudrate}")
        # In order to allow non root user to access port, execute "sudo usermod -a -G dialout <your_username>" once.
        try:
            self.client = ModbusSerialClient(method='rtu', port=port, baudrate=baudrate, timeout=1)
        except Exception as exc:
            print(f"Error defining modbus port {port}, speed={baudrate}")
            print(f"Received exception {exc}")
            return True
        try:
            self.client.connect()
        except pymodbus.ModbusException as exc:
            print(f"Error connecting to modbus port {port}, speed={baudrate}")
            print(f"Received ModbusException({exc}) from library")
            return True
        except Exception as exc:
            print(f"Error connecting to modbus port {port}, speed={baudrate}")
            print(f"Received exception {exc}")
            return True

        if not self.client.is_socket_open():
            print(f"Error connecting to modbus port {port}, speed={baudrate}")
            print("Socket is closed")
            return True

        if self.traceDebug:
            print(F"Connected to modbus port {port}, speed={baudrate}")
        return False