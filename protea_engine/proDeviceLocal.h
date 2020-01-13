#ifndef _PRO_DEVICE_LOCAL_H
#define _PRO_DEVICE_LOCAL_H

/** @file proDeviceLocal.h
 \brief  interface to local device driver adapters 
*/

#include "proIo.h"
#include "proDevice.h"
#include <string>

//--- class DeviceLocal --------------------------------------------

/// a device connecting to a local device driver adapter application
class DeviceLocal : public DeviceInput {
public:
	/// creates a local device adapter by calling an external command
	static DeviceInput * create(const std::string & cmd);
	/// destructor
	virtual ~DeviceLocal();
	/// updates device state
	virtual int update(double deltaT=0.0);
	/// returns class specific type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
protected:
	/// constructor
	DeviceLocal(const std::string & deviceName, const char** arg, unsigned int nAxes, unsigned int nButtons);
	/// shared memory to communicate with input device adapter application
	SharedMemory m_shm;
};

#endif // _PRO_DEVICE_LOCAL_H
