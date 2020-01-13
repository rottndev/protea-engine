// DeviceInputTest protea example application
// started 2009-01-17 by Gerald Franz
// current version 2009-07-20

#ifdef _USE_SDL
#  include "DeviceSdl.h"
#else
#  include <proGlfw.h>
#  include <protea.h>
#endif
#include "defaultFont.xpm"
#include <GL/gl.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>
using namespace std;

//--- main function ------------------------------------------------
int main( int argc, char **argv ) {
	// determine application path for resource loading:
	string appPath;
	if(argc) {
		string s(argv[0]);
#if defined WIN32 || defined _WIN32
		char delimiter='\\';
#else
		char delimiter='/';
#endif
		appPath=s.substr(0,s.rfind(delimiter)+1);
	}
	// parse command line options:
	int winSizeX=640;
	int winSizeY=480;
	bool fullScreen=false;
	int joystickId = -1;
	char * localDeviceName = 0;
	for(int i=0; i<argc; ++i) {
		//printf("arg %i: [%s]\n",i,argv[i]); fflush(stdout);
		size_t argsz = strlen(argv[i]);
		if(argsz<2) continue;
		if(argv[i][0]=='-') switch(argv[i][1]) {
		case 'h': printf("usage: %s [-h(elp)] [-jN(input from joystick n)] [-lDeviceName (input from local device)] [-f(ullscreen)] [-x(sizeX)] [-y(sizeY)]\n", argv[0]); return 0;
		case 'j': joystickId = atoi(&argv[i][2]); break;
		case 'l': localDeviceName = &argv[i][2]; break;
		case 'f': fullScreen =true; break;
		case 'x': winSizeX = atoi(&argv[i][2]); break;
		case 'y': winSizeY = atoi(&argv[i][2]); break;
		default: printf("Unknown command line switch \"%s\"\n", argv[i]);
		}
	}
			
	// open devices:
	DeviceWindow * pWnd = DeviceWindowGlfw::create(winSizeX,winSizeY,fullScreen);
	if(!pWnd) return 1;
	pWnd->title("testInput");
	DeviceInput * pDev = localDeviceName ? DeviceLocal::create(localDeviceName) : 0;
	if(!pDev&&joystickId>= 0) pDev = DeviceJoystickGlfw::create(joystickId);
	DeviceInput & devInput = pDev ? *pDev : *DeviceInput::instance(0);
	TimerGlfw timer;

	// initialize GL:
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glViewport (0, 0, pWnd->width(), pWnd->height());
	Font * pFont = FontMap::create(*Image::createFromXPM(defaultFont));
	if(!pFont) return 1;
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// initialize basic application variables:
	unsigned int nAxes = devInput.nAxes();
	unsigned int nButtons = devInput.nButtons();
	float * axis = new float[nAxes];
	char s[256];

	float hLine = pFont->height();
	float wRect = (pWnd->width()-2.0f*hLine) /nAxes;
	float y1 = pWnd->height() - 2.5f * hLine;
	float lSquare = (pWnd->width()-2.0f*hLine)/16;
	float y3 = ceil(nButtons/16.0f)*lSquare;
	float y0 = y3+1.5f*hLine;
	
    while(pWnd->open()) { // main loop:
		// update input:
		devInput.axes(axis);
		timer.update();
				
		// update visualization:
		glClear (GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,pWnd->width(),0,pWnd->height(),-1.0f,1.0f);
		glMatrixMode(GL_MODELVIEW);
		
		// display axes:
		glColor4f(0.0f,0.5f,0.0f,1.0f);
		glBegin(GL_QUADS);
		for(unsigned int i=0; i<nAxes; ++i) {
			glVertex2f(hLine+i*wRect+1, 0.5f*(y0+y1));
			glVertex2f(hLine+(i+1)*wRect-1, 0.5f*(y0+y1));
			glVertex2f(hLine+(i+1)*wRect-1, 0.5f*(y0+y1)+0.5f*(y1-y0)*axis[i]);
			glVertex2f(hLine+i*wRect+1,  0.5f*(y0+y1)+0.5f*(y1-y0)*axis[i]);
		}
		glEnd();
		glColor4f(0.0f,1.0f,0.0f,1.0f);
		for(unsigned int i=0; i<nAxes; ++i) {
			glBegin(GL_LINE_LOOP);
			glVertex2f(hLine+i*wRect+1, y0);
			glVertex2f(hLine+(i+1)*wRect-1, y0);
			glVertex2f(hLine+(i+1)*wRect-1, y1);
			glVertex2f(hLine+i*wRect+1, y1);
			glEnd();
		}
		
		// display buttons:
		glColor4f(0.0f,0.5f,0.0f,1.0f);
		glBegin(GL_QUADS);
		for(unsigned int i=0; i<nButtons; ++i) if(devInput.button(i)){
			glVertex2f(hLine+(i%16)*lSquare+1, y3-lSquare+2 - floor(i/16.0f)*lSquare);
			glVertex2f(hLine+(i%16+1)*lSquare-1, y3-lSquare+2 - floor(i/16.0f)*lSquare);
			glVertex2f(hLine+(i%16+1)*lSquare-1, y3 - floor(i/16.0f)*lSquare);
			glVertex2f(hLine+i%16*lSquare+1, y3 - floor(i/16.0f)*lSquare);
		}
		glEnd();
		glColor4f(0.0f,1.0f,0.0f,1.0f);
		for(unsigned int i=0; i<nButtons; ++i) {
			glBegin(GL_LINE_LOOP);
			glVertex2f(hLine+(i%16)*lSquare+1, y3-lSquare+2 - floor(i/16.0f)*lSquare);
			glVertex2f(hLine+(i%16+1)*lSquare-1, y3-lSquare+2 - floor(i/16.0f)*lSquare);
			glVertex2f(hLine+(i%16+1)*lSquare-1, y3 - floor(i/16.0f)*lSquare);
			glVertex2f(hLine+i%16*lSquare+1, y3 - floor(i/16.0f)*lSquare);
			glEnd();
		}

		// texts:
		glEnable (GL_BLEND);
		glEnable (GL_TEXTURE_2D );
		pFont->print(hLine+1,y1, "Axes:");
		for(unsigned int i=0; i<nAxes; ++i) {
			sprintf(s,"%i",i);
			pFont->print(hLine+i*wRect+2,y0+2, s);
			sprintf(s,"%.2f",axis[i]);
			pFont->print(hLine+(i+0.5f)*wRect - 0.5f*pFont->width(s), 0.5f*(y0+y1) - 0.5f*hLine, s);
		}

		sprintf(s,"%s device %i \"%s\"", devInput.type(), devInput.id(), devInput.name().c_str());
		s[0]=toupper(s[0]);
		pFont->print(0.5f*pWnd->width()-0.5f*pFont->width(s), pWnd->height()-hLine, s);

		
		pFont->print(hLine+1,y3, "Buttons:");
		for(unsigned int i=0; i<nButtons; ++i){
			sprintf(s,"%i",i);
			pFont->print(hLine+(i%16)*lSquare+2, y3-lSquare+3 - floor(i/16.0f)*lSquare, s);
		}
		
		glDisable (GL_TEXTURE_2D );
		glDisable (GL_BLEND);

		DeviceInput::updateAll(timer.deltaT());
		timer.sleep(.01); // do not block the CPU
	}
	delete pDev;
	delete pFont;
	delete pWnd;
	delete [] axis;
    return 0;
}
