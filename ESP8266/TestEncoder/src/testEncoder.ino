#include <../../constants.h>
#include <rotationSensor.h>

RotationSensor encoder(offsetAngle, rxPin, txPin, traceDebug);
uint16_t resultValue;

Modbus::ResultCode readSuccess(Modbus::FunctionCode fc, const Modbus::RequestData data) {
  float angle = encoder.computeAngle(resultValue);
  if (traceDebug) {
    Serial.printf("Angle: %f\n", angle);
  }
  return Modbus::EX_SUCCESS;
}

Modbus::ResultCode readError(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event != Modbus::EX_SUCCESS) {
    if (traceDebug) {
      Serial.printf("Read error: 0x%x\n", event);
    }
  }
  return Modbus::EX_SUCCESS;
}

void setup() {
  Serial.begin(74880);
  Serial.printf("Test encoder\n");
  encoder.setErrorCb(readError);
  encoder.setSuccessCb(readSuccess);
  encoder.begin();
}

void loop() {
  encoder.getAngle(resultValue);
  delay(1000);
}