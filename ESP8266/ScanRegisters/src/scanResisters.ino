#include <../../constants.h>
#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define SLAVE_ID 1
#define FIRST_REG 0
#define REG_COUNT 1

SoftwareSerial S(rxPin, txPin);
ModbusRTU mb;

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Callback to monitor errors
  if (event != Modbus::EX_SUCCESS) {
    Serial.printf("error: 0x%x ", event);
  }
  return true;
}

void setup() {
  Serial.begin(74880);
  Serial.printf("Scan registers: slave: %d, register %d, count: %d\n", SLAVE_ID, FIRST_REG, REG_COUNT);
  S.begin(9600, SWSERIAL_8N1);
  mb.begin(&S);
  mb.master();
}

void loop() {
  uint16_t res;
  for (int16_t i=0; i<REG_COUNT; i++) {
    Serial.printf("Register %d: ", i);
    if (!mb.slave()) {    // Check if no transaction in progress
      mb.readHreg(SLAVE_ID, i, &res, 1, cb); // Send Read Hreg from Modbus Server
      while(mb.slave()) { // Check if transaction is active
        mb.task();
        delay(10);
      }
      Serial.printf("value: 0x%x\n", res);
    }
  }
  delay(1000);
}