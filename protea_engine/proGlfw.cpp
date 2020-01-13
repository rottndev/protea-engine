#include "proGlfw.h"
#include "proResource.h"
#include <cstdio>
using namespace std;

#ifndef GL_CLAMP_TO_EDGE
#  define GL_CLAMP_TO_EDGE 0x812F
#endif

unsigned int glfwInitialized = 0;
    
//--- class DeviceWindowGlfw -----------------------------------

DeviceWindow * DeviceWindowGlfw::create(unsigned int sizeX, unsigned int sizeY, bool fullScreen, bool stencilBuffer) {
	if(DeviceWindow::sp_instance) return sp_instance;
	if(!glfwInitialized++) glfwInit();
	if(!sizeX||!sizeY) {
		GLFWvidmode desktopMode;
		glfwGetDesktopMode(&desktopMode);
		sizeX=desktopMode.Width;
		sizeY=desktopMode.Height;
	}
	if(!glfwOpenWindow(sizeX, sizeY, 8,8,8,0, 24, 0, fullScreen ? GLFW_FULLSCREEN : GLFW_WINDOW )) {
		fprintf(stderr, "DeviceWindow::create() ERROR: cannot initialize GLFW video.\n");
		return 0;
	}
	sp_instance = new DeviceWindowGlfw(sizeX,sizeY, fullScreen, stencilBuffer);
	sp_instance->	initGraphics();
	return sp_instance;
}

DeviceWindowGlfw::DeviceWindowGlfw(unsigned int width, unsigned int height, bool fullScreen, bool stencilBuffer) : DeviceWindow(width, height, fullScreen),
	m_clearBits(stencilBuffer ? (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT) : (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)) {
	m_nAxes = 6;
	m_axes = new float[m_nAxes];
	memset(m_axes,0,sizeof(float)*m_nAxes);
	m_nButtons = 96;
	m_textInput[0]=0;
	m_name = "window";
}

DeviceWindowGlfw::~DeviceWindowGlfw() {
	glfwCloseWindow();
	sp_instance = 0;
	--glfwInitialized;
	if(!glfwInitialized) glfwTerminate();
}

void DeviceWindowGlfw::initGraphics() {
	glViewport(0, 0, m_w, m_h);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glClear (m_clearBits);
	glClearColor (m_clearColor[0], m_clearColor[1], m_clearColor[2], 0.0f);
	glfwSwapInterval(1); // enable vertical sync
	glfwSetMousePos(m_w/2, m_h/2); // reset to middle position
	glfwSetMouseWheel(0);
	glfwDisable( GLFW_STICKY_KEYS );
	glfwSetWindowCloseCallback( cbClose );
	glfwSetWindowSizeCallback( cbResize );
	glfwSetKeyCallback( cbKeyInput );
	glfwSetCharCallback( cbTextInput );
	m_now = glfwGetTime();
}

int DeviceWindowGlfw::update(double deltaT) {
	if(!m_open) return -1;
	DeviceWindow::update(deltaT);
	glfwSetMouseWheel(0);
	glFlush();
	glfwSwapBuffers();   // swap buffers and update events
	glClear (m_clearBits);
	m_deltaT = glfwGetTime()-m_now;
	m_now+=m_deltaT;

	// check for error conditions:
	GLenum gl_error = glGetError(); // this appears mandatory to get smooth animations...?
	if(gl_error&&m_open) fprintf(stderr, "OpenGL ERROR: %i\n", gl_error);

	// update input:
	if(glfwGetKey(GLFW_KEY_LEFT)) m_axes[0]=-1.0f;
	else if(glfwGetKey(GLFW_KEY_RIGHT)) m_axes[0]=1.0f;
	else m_axes[0]=0.0f;
	if(glfwGetKey(GLFW_KEY_UP)) m_axes[1]=1.0f;
	else if(glfwGetKey(GLFW_KEY_DOWN)) m_axes[1]=-1.0f;
	else m_axes[1]=0.0f;
	if(glfwGetKey(GLFW_KEY_PAGEUP)) m_axes[2]=1.0f;
	else if(glfwGetKey(GLFW_KEY_PAGEDOWN)) m_axes[2]=-1.0f;
	else m_axes[2]=0.0f;
	int mouseX, mouseY;
	glfwGetMousePos( &mouseX, &mouseY );
	if(m_cursorRestricted) if((mouseX<0)||(mouseY<0)||(mouseX+1>(int)m_w)||(mouseY+1>(int)m_h)) {
		mouseX = mouseX<0 ? 0 : mouseX+1>(int)m_w ? m_w-1 : mouseX;
		mouseY = mouseY<0 ? 0 : mouseY+1>(int)m_h ? m_h-1 : mouseY;
		glfwSetMousePos( mouseX, mouseY );
	}
	m_axes[3]= (float(mouseX)/float(m_w)-0.5f)*2.0f;
	m_axes[4]=-(float(mouseY)/float(m_h)-0.5f)*2.0f;
	m_axes[5]=static_cast<float>(glfwGetMouseWheel());

	m_buttonPrev = m_buttonCurr;
	m_buttonCurr = 0;
	if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)||glfwGetKey(GLFW_KEY_F1))
	m_buttonCurr|=1;
	if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)||glfwGetKey(GLFW_KEY_F2))
	m_buttonCurr|=(1<<1);
	if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE)||glfwGetKey(GLFW_KEY_F3))
	m_buttonCurr|=(1<<2);
	if(glfwGetKey(GLFW_KEY_F4))  m_buttonCurr|=(1<<3);
	if(glfwGetKey(GLFW_KEY_F5)||(m_axes[5]<0.0f))  m_buttonCurr|=(1<<4);
	if(glfwGetKey(GLFW_KEY_F6)||(m_axes[5]>0.0f))  m_buttonCurr|=(1<<5);
	if(glfwGetKey(GLFW_KEY_F7))  m_buttonCurr|=(1<<6);
	if(glfwGetKey(GLFW_KEY_F8))  m_buttonCurr|=(1<<7);
	if(glfwGetKey(GLFW_KEY_F9))  m_buttonCurr|=(1<<8);
	if(glfwGetKey(GLFW_KEY_F10)) m_buttonCurr|=(1<<9);
	if(glfwGetKey(GLFW_KEY_F11)) m_buttonCurr|=(1<<10);
	if(glfwGetKey(GLFW_KEY_F12)) m_buttonCurr|=(1<<11);
	if(glfwGetKey(GLFW_KEY_ESC)) m_buttonCurr|=(1<<KEY_ESC);
	if(glfwGetKey(GLFW_KEY_ENTER)||glfwGetKey(GLFW_KEY_KP_ENTER))
		m_buttonCurr|=(1<<KEY_ENTER);
	if(glfwGetKey(GLFW_KEY_TAB)) m_buttonCurr|=(1<<KEY_TAB);

	if((glfwGetKey(GLFW_KEY_LCTRL)||glfwGetKey(GLFW_KEY_RCTRL))&&glfwGetKey('C')) // CTRL-C
		sp_instance->close();
    
	return gl_error;
}

void DeviceWindowGlfw::clearColor(float r, float g, float b) {
	m_clearColor[0]=r;
	m_clearColor[1]=g;
	m_clearColor[2]=b;
	glClearColor (m_clearColor[0], m_clearColor[1], m_clearColor[2], 0.0f);
}

void DeviceWindowGlfw::fullScreen(bool fullscreen) {
	if(m_fullscreen==fullscreen) return;
	glfwCloseWindow();
	if(fullscreen) { // go fullscreen
		GLFWvidmode desktopMode;
		glfwGetDesktopMode(&desktopMode);
		if(!glfwOpenWindow(desktopMode.Width, desktopMode.Height, 8,8,8,0, 24, 0, GLFW_FULLSCREEN)) {
			fprintf(stderr, "DeviceWindow::fullScreen() ERROR: cannot initialize GLFW video.\n");
			m_open=false;
			return;
		}
		title(m_title);
		m_w = desktopMode.Width;
		m_h = desktopMode.Height;
		m_fullscreen=true;
	}
	else { // switch to window mode
		if(!glfwOpenWindow(m_wOriginal, m_hOriginal, 8,8,8,0, 24, 0, GLFW_WINDOW)) {
			fprintf(stderr, "DeviceWindow::fullScreen() ERROR: cannot initialize GLFW video.\n");
			m_open=false;
			return;
		}
		title(m_title);
		m_w = m_wOriginal;
		m_h = m_hOriginal;
		m_fullscreen=false;
	}
	initGraphics();
	for(vector<ListenerResize*>::iterator it = mv_listenerResize.begin(); it!=mv_listenerResize.end(); ++it)
		(*it)->resize(static_cast<float>(m_w), static_cast<float>(m_h));
}

void DeviceWindowGlfw::cursorVisible(bool visible) { 
	if(visible) glfwEnable( GLFW_MOUSE_CURSOR ); 
	else glfwDisable( GLFW_MOUSE_CURSOR );
	m_cursorVisible = visible;
}

bool DeviceWindowGlfw::button(unsigned int n) const {
	if(n<32) return DeviceInput::button(n);
	if((n>=32)&&(n<m_nButtons)) return glfwGetKey(n)!=0;
	return false;
}

void DeviceWindowGlfw::resize(  int  width,  int  height  ) {
	m_w = width;
	m_h = height;
	for(vector<ListenerResize*>::iterator it = mv_listenerResize.begin(); it!=mv_listenerResize.end(); ++it)
		(*it)->resize((float)width, (float)height);
	glViewport(0, 0, width, height);
}

void DeviceWindowGlfw::processKeyInput( int key, int action ) {
    if(action!=GLFW_PRESS) return;
    if(((key==GLFW_KEY_ENTER)||(key==GLFW_KEY_KP_ENTER))&&m_textInputSize) 
		m_textInputComplete=true;
    else if((key==GLFW_KEY_BACKSPACE)&&m_textInputSize) m_textInput[--m_textInputSize]=0;
}

void DeviceWindowGlfw::processTextInput( int character, int action ) {
    if((action!=GLFW_PRESS)||(character>255)||(m_textInputSize+2>sc_textInputMax)) return;
    m_textInput[m_textInputSize]=static_cast<unsigned char>(character);
	m_textInput[++m_textInputSize]=0;
}

unsigned int DeviceWindowGlfw::createTexture(const Image& img, const std::string & name, unsigned int texFlags) {
	// check if image needs to be upsampled
	unsigned char* data = 0;
	unsigned int width = img.width();
	unsigned int height = img.height();
	if(!glfwExtensionSupported("GL_ARB_texture_non_power_of_two")) {
		int maxSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize);
		width=1;
		while((width<img.width())&&(width<(unsigned int)maxSize)) width<<=1; // calculate next larger 2^N width
		height=1;
		while((height<img.height())&&(height<(unsigned int)maxSize)) height<<=1; // calculate next larger 2^N width
		if((width!=img.width()) || (height!=img.height()))
			data=img.upsample(width, height);
	}
	// upload texture:
	unsigned int id;
	glGenTextures(1,&id);
	glBindTexture (GL_TEXTURE_2D, id);
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (texFlags&TEXFLAG_REPEAT) ? GL_REPEAT : (texFlags&TEXFLAG_CLAMP_TO_EDGE) ? GL_CLAMP_TO_EDGE : GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (texFlags&TEXFLAG_REPEAT) ? GL_REPEAT : (texFlags&TEXFLAG_CLAMP_TO_EDGE) ? GL_CLAMP_TO_EDGE : GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	GLenum fmt = img.depth() == 1 ? ((texFlags&TEXFLAG_8BIT_ALPHA) ? GL_ALPHA : GL_LUMINANCE) : 
		img.depth()==2 ? GL_LUMINANCE_ALPHA : img.depth()==3 ? GL_RGB : GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, data ? data : img.data());
	mm_tex[name]=new TexProperty(id, img.width(), img.height(), img.depth());
	free(data);
	return id;
}

Font* DeviceWindowGlfw::createFont(const Image & img, const std::string & name) { 
	Font* pFont = FontMap::create(img); 
	if(!pFont) return 0;
	if(name.size()) mm_font[name] = pFont;
	if(!mp_fontDefault||!name.size()||(name=="default")) mp_fontDefault = pFont;
	return pFont;
}

//--- class DeviceJoystickGlfwGlfw  ------------------------------------

const char* DeviceJoystickGlfw::s_type = "joystick";

DeviceInput * DeviceJoystickGlfw::create(int joystickId) {
	if(!isAvailable(joystickId)) return 0;
	// check if joystick is already open:
	for(unsigned int i=0; DeviceInput::instance(i)!=0; ++i) {
		DeviceJoystickGlfw* pJoy = dynamic_cast<DeviceJoystickGlfw*>(DeviceInput::instance(i));
		if(pJoy&&(pJoy->m_id==joystickId)) return pJoy;
	}
	return new DeviceJoystickGlfw(joystickId); 
}

DeviceJoystickGlfw::DeviceJoystickGlfw(int joystickId) : DeviceInput(), m_id(joystickId) {
	if(!glfwInitialized++) glfwInit();
	m_nAxes = glfwGetJoystickParam(m_id,GLFW_AXES);
	if(m_nAxes) {
		m_axes = new float[m_nAxes];
		memset(m_axes,0,sizeof(float)*m_nAxes);
	}
	m_nButtons = glfwGetJoystickParam(m_id,GLFW_BUTTONS);
	if(m_nButtons>sizeof(unsigned int)*8)
		m_nButtons = sizeof(unsigned int)*8;
	m_name="game controller 0";
	m_name[m_name.size()-1]='0'+m_id;
}

DeviceJoystickGlfw::~DeviceJoystickGlfw() {
	--glfwInitialized;
	if(!glfwInitialized) glfwTerminate();
}

int DeviceJoystickGlfw::update(double deltaT) {
	glfwGetJoystickPos(m_id,m_axes,m_nAxes);	
	m_buttonPrev = m_buttonCurr;
	m_buttonCurr = 0;
	unsigned char buttons[32];
	glfwGetJoystickButtons(m_id,buttons,m_nButtons);
	for(unsigned int i=0; i<m_nButtons; ++i)
		if(buttons[i]) m_buttonCurr|=(1<<i);
	return 0;
}

bool DeviceJoystickGlfw::isAvailable(unsigned int joystickId) {
	if(!glfwInitialized) glfwInit();
	bool ret = (glfwGetJoystickParam(joystickId,GLFW_PRESENT)!=0);
	if(!glfwInitialized) glfwTerminate();
	return ret;
}
