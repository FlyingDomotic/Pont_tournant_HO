#ifndef StepperCommand_h
	#define StepperCommand_h

	#include <inttypes.h>

	#ifdef __cplusplus
		class StepperCommand {
			public:
				StepperCommand(uint8_t _pulsePin, uint8_t _directionPin, uint8_t _enablePin, float _degreesPerStep, uint16_t _microStepsPerStep, uint8_t _driverMinimalMicroSec, bool _traceDebug);
				void begin(float _RPM = 0.0);
				void setRPM(float _RPM);
				void rotateAngle(float _angle);
				unsigned long microStepsForAngle(float _angle);
				float anglePerMicroStep(void);
				void turnOneMicroStep(void);

			private:
				float degreesPerStep;						// Motor's number of degrees per step
				float stepDuration;							// Step duration in seconds
				uint16_t microStepsPerStep;					// Drivers number of micro-steps per step
				uint8_t driverMinimalMicroSec;				// Drivers minimal signal duration in micro-seconds
				float requiredRPM;							// Required motor RPM
				bool traceDebug;							// Trace debug messages?
				uint8_t pulsePin;							// Driver's pulse Pin
				uint8_t directionPin;						// Driver's direction Pin
				uint8_t enablePin;							// Drivers enable Pin
		};
	#endif
#endif
