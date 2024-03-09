#ifndef RotationSensor_h
	#define RotationSensor_h

	#include <inttypes.h>
	#include <ModbusRTU.h>
	#include <SoftwareSerial.h>

	#ifdef __cplusplus
		class RotationSensor {
			public:
				RotationSensor(float _offsetAngle, uint8_t _rxPin,  uint8_t _txPin, bool _traceDebug);
				void begin(void);
				void loop(void);
				bool active(void);
				RotationSensor& setErrorCb(std::function<Modbus::ResultCode(Modbus::ResultCode event, uint16_t transactionId, void* data)> errorCb);
				RotationSensor& setSuccessCb(std::function<Modbus::ResultCode(Modbus::FunctionCode fc, const Modbus::RequestData data)> successCb);
				void getAngle(uint16_t result);
				float computeAngle(uint16_t valueRead);

			private:
				std::function<Modbus::ResultCode(Modbus::ResultCode event, uint16_t transactionId, void* data)> errorCb;
				std::function<Modbus::ResultCode(Modbus::FunctionCode fc, const Modbus::RequestData data)> successCb;
				float floatModulo(float f, float modulo);
				float offsetAngle;
				uint8_t rxPin;
				uint8_t txPin;
				bool traceDebug;
				SoftwareSerial modbusSerial;
				ModbusRTU modbusRtu;
		};
	#endif
#endif
