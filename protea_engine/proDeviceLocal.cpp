//--- class DeviceLocal --------------------------------------------

#include "proStr.h"
#include "proDeviceLocal.h"

using namespace std;

const char* DeviceLocal::s_type = "local";

DeviceInput * DeviceLocal::create(const std::string & cmd) {
	const char* deviceName = cmd.c_str();
	const char* argv[2];
	char** arg = 0;
	char* argstr = 0;
	DeviceInput* pDev = 0;
	// look for arguments as part of command line:
	if(cmd.find(' ')<cmd.size()) {
		argstr = (char*)malloc(cmd.size()+1);
		strcpy(argstr,cmd.c_str());
		arg = tokenize(argstr);
		deviceName = arg[0];
	}
	// check presence of device (adapter):
	argv[0]="-qd";
	argv[1]=0;
	int nDevices=io::spawn(deviceName,argv,1);
	if(nDevices<0)
		fprintf(stderr, "ERROR: Opening local device adapter \"%s\" failed.\n", deviceName);
	else if(!nDevices)
		fprintf(stderr, "ERROR: Local device \"%s\" not present.\n", deviceName);
	else { // query device properties:
		argv[0]="-qa";
		unsigned int nAxes=io::spawn(deviceName,argv,1);
		argv[0]="-qb";
		unsigned int nButtons=io::spawn(deviceName,argv,1);
		//fprintf(stderr, "%s nDevices:%i nAxes:%u nButtons:%u.\n", deviceName, nDevices, nAxes, nButtons);	fflush(stderr);
		// create device:
		pDev = new DeviceLocal(deviceName, const_cast<const char**>(arg), nAxes, nButtons);
		if(!pDev->nAxes()&&!pDev->nButtons()) {
			delete pDev;
			pDev=0;
		}
	}
	free(arg);
	free(argstr);
	return pDev;
}

DeviceLocal::DeviceLocal(const std::string & deviceName, const char** arg, unsigned int nAxes, unsigned int nButtons) : DeviceInput() {
	const char* argv[1];
	argv[0]=0;
	io::spawn(deviceName.c_str(), arg ? arg : argv, 0);
	SharedMemory_open(&m_shm, deviceName.c_str(), nAxes*sizeof(float)+sizeof(unsigned int));
	if(!m_shm.data) return;
	m_nAxes = nAxes;
	m_nButtons = nButtons;
	m_name = deviceName;
	m_axes = new float[m_nAxes];
	memset(m_axes,0,sizeof(float)*m_nAxes);
}

DeviceLocal::~DeviceLocal() {
    if(m_shm.data) SharedMemory_close(&m_shm);
}

int DeviceLocal::update(double deltaT) {
	if(!m_shm.data||(*m_shm.pCounter<2)) return -1;
	// read from device:
	memcpy(m_axes, ((float*)m_shm.data)+1, sizeof(float)*m_nAxes);
	m_buttonPrev = m_buttonCurr;
	m_buttonCurr = *(unsigned int*)m_shm.data;
	return 0;
}

