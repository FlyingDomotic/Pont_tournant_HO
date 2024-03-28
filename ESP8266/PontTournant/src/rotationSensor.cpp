#include <Arduino.h>
#include <rotationSensor.h>
#include <ModbusRTU.h>

// Class initialization 
//   input:
//     _offsetAngle               // Angle of zero degrees position (depends on hardware)
//     _rxPin                     // Modbus RX pin
//     _txPin                     // Modbus TX pin
//     _traceDebug                // Trace debug messages?
RotationSensor::RotationSensor(uint8_t _rxPin,  uint8_t _txPin, bool _traceDebug) {
  rxPin = _rxPin;                 // Modbus RX pin
  txPin = _txPin;                 // Modbus TX pin
  traceDebug = _traceDebug;       // Trace debug messages?
}

//  Initialize modbus communication
void RotationSensor::begin(void){
  modbusSerial.begin(9600, SWSERIAL_8N1, rxPin, txPin);
  modbusRtu.begin(&modbusSerial);
  modbusRtu.master();
}

//  Set parameters
void RotationSensor::setParams(float _offsetAngle, bool _traceDebug) {
  offsetAngle = _offsetAngle;     // Angle of zero degrees position (depends on hardware)
  traceDebug = _traceDebug;       // Trace debug messages?
}

// Loop for this module
void RotationSensor::loop (void) {
  if (modbusRtu.slave()) {
    modbusRtu.task();
  }
}

bool RotationSensor::active(void) {
  return modbusRtu.slave();
}
// Ask modbus sensor for motor angle. Returned value is in degrees, corrected by an offset.
void RotationSensor::getAngle(uint16_t *result){
  if (traceDebug) {
      Serial.print("Reading angle: ");
  }
  if (!modbusRtu.slave()) {                         // Check if no transaction in progress
    if (callback) {
      modbusRtu.readHreg(1, 0, result, 1, callback); // Send Read Hreg from Modbus Server
    } else {
      modbusRtu.readHreg(1, 0, result, 1, 0);       // Send Read Hreg from Modbus Server
    }
  }
}

// Convert a value read in modbus register 0 to an angle in degrees, giving an initial offset
//   input:
//       data: device data to convert to angle
//   returns:
//       angle, in degrees, corresponding to data
float RotationSensor::computeAngle(uint16_t valueRead) {
    return floatModulo(((360.0 * valueRead) / 65536.0) - offsetAngle,  360.0);
}

RotationSensor& RotationSensor::setCallback(std::function<Modbus::ResultCode(Modbus::ResultCode event, uint16_t transactionId, void* data)> callback) {
  this->callback = callback;
  return *this;
}

// Simulate modulo for floats (efficient if float id close to modulo)
float RotationSensor::floatModulo (float f, float modulo) {
  if (f >= 0.0) {
    while (f >= modulo) {
      f -= modulo;
    }
  } else {
    while (f >= 0.0) {
      f += modulo;
    }
  }
  return f;
}
