uint8_t enablePin = D0;                 // Pin where stepper enable signal is connected to
uint8_t pulsePin = D1;                  // Pin where stepper pulse signal is connected to
uint8_t directionPin = D2;              // Pin where stepper direction signal is connected to
uint8_t rxPin = D5;                     // Pin where RS485 RX signal is connected to
uint8_t txPin = D6;                     // Pin where RS485 TX signal is connected to
float degreesPerStep = 1.8;             // Motor's number of degrees per step
float microStepsPerStep = 2.0;          // Drivers number of micro-steps per step
uint16_t driverMinimalMicroSec = 3;     // Drivers minimal signal duration in micro-seconds
float requiredRPM = 2;                  // Required motor RPM
bool traceDebug = true;                 // Trace debug messages?
uint8_t maxDelta = 3;                   // Maximum percentage delta around reference point click
float offsetAngle = 0.0;                // Angle of zero degrees position (depends on hardware)
uint8_t slaveId = 1;                    // Slave Id
uint8_t regId = 0;                      // Register Id
// Angles of each index
#define POSITION_COUNT 15
float indexes[POSITION_COUNT] = {0.0, 30.0, 60.0, 75.0, 90.0, 105.0, 120.0, 135.0, 150.0, 165.0, 180.0, 195.0, 210.0, 225.0, 270.0};
// Relative position of each index on Web server image
float xPos[POSITION_COUNT] = {91.7, 86.2, 68.0, 57.7, 47.2, 34.7, 25.7, 18.0, 11.7, 9.5, 8.7, 11.0, 15.2, 21.2, 50.7};
float yPos[POSITION_COUNT] = {51.4, 27.9, 10.1, 6.5, 5.4, 8.1, 13.7, 20.5, 31.7, 41.6, 52.6, 63.8, 73.3, 81.1, 94.4};


