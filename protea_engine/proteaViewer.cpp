#include "protea.h"
#include "skydome.h"
#include "proGlfw.h"
#include "proIoWrl.h"
#include "proIoObj.h"
#include "proIo3ds.h"
#include "proIoPng.h"
#include "proIoJpg.h"
#include <proCanvas.h>
#include <proGui.h>

#include <cstdio>
#include <iostream>
using namespace std;

//--- class Application --------------------------------------------

class Application : public Callable {
public:
	/// constructor
	Application(proScene & scene, proLight & sun) : m_scene(scene), m_sun(sun), m_wireframe(false), m_groundPlane(true), m_shadow(true) { }
	/// generic method calling the object instance to evalute the provided commands
	virtual Var call(const std::string & cmd, const Var & arg);
	/// returns all keys/command names provided by this Callable as Var::ARRAY
	virtual Var info() const {
		return Var().append("about").append("msgbox").append("load").append("clear")
			.append("wireframe").append("groundplane").append("shadow"); }
	/// updates application
	int update(double deltaT) { return 0; }
	/// returns and clears current application message
	std::string message() { std::string ret=m_msg; m_msg.clear(); return ret; }

	/// loads scene
	bool load(const std::string & filename);
	/// clears scene
	void clear() { m_scene.erase(&m_sun, false); m_scene.clear(); m_scene.append(&m_sun, false); m_msg = "Scene cleared"; }

	/// returns true if visualization shall be done in wireframe mode
	bool drawWireFrame() const { return m_wireframe; }
	/// turns wireframe mode on/off
	void drawWireFrame(bool yesno){ 
		m_wireframe=yesno; 
		m_msg="Wireframe "+string(m_wireframe ? "on":"off");
	}
	/// returns true if a ground plane shall be displayed
	bool drawGroundPlane() const { return m_groundPlane; }
	/// turns ground plane on/off
	void drawGroundPlane(bool yesno){ 
		m_groundPlane=yesno; 
		m_msg="Ground plane "+string(m_groundPlane? "on":"off");
	}
	/// returns true if shadows shall be displayed
	bool drawShadow() const { return m_shadow; }
	/// turns shadows on/off
	void drawShadow(bool yesno){ 
		m_shadow=yesno; 
		m_msg="Shadows "+string(m_shadow? "on":"off");
	}
protected:
	/// scene reference
	proScene & m_scene;
	/// primary light source reference
	proLight & m_sun;
	/// polygon mode
	bool m_wireframe;
	/// flag turning ground plane on/off
	bool m_groundPlane;
	/// flag turning shadows on/off
	bool m_shadow;
	/// message string
	std::string m_msg;
};

Var Application::call(const std::string & cmd, const Var & arg ) {
	if(cmd=="about")
		return cmdLine::name()+' '+cmdLine::version()+' '+cmdLine::date()+" by "+cmdLine::author()+".\n"+cmdLine::shortDescr();
	else if(cmd=="load")
		return load(arg[0].string());
	else if(cmd=="clear")
		clear();
	else if(cmd=="wireframe") {
		if(arg[0].type()) drawWireFrame(arg[0].boolean());
		return m_wireframe;
	}
	else if(cmd=="groundplane") {
		if(arg[0].type()) drawGroundPlane(arg[0].boolean());
		return m_groundPlane;
	}
	else if(cmd=="shadow") {
		if(arg[0].type()) drawShadow(arg[0].boolean());
		return m_shadow;
	}
	return Var::null;
}

bool Application::load(const std::string & filename) {
	proNode* pScenery = ModelMgr::singleton().load(filename);
	if(!pScenery) return false;
	m_scene.append(pScenery,false);
	pScenery->initGraphics();
	m_msg="Scene \""+filename+"\" loaded.";		
	return true;
}

//--- class SkyController ------------------------------------------

/// a dynamic controller for the sky dome and cloud layer
class SkyController : public Callable {
public:
	/// constructor
	SkyController(SkyDome & sky, CloudLayer & clouds, proLight & sun) : m_sky(sky), m_clouds(clouds), m_sun(sun) { }
	/// generic method interpreting and evaluting the provided command
	virtual Var call(const std::string & cmd, const Var & arg);
	/// returns all keys/command names provided by this callable as Var::ARRAY
	virtual Var info() const {
		return Var().append("cloudsVisible").append("cloudVelocity").append("sunPosition").append("turbidity"); }
protected:
	/// sky dome reference
	SkyDome & m_sky;
	/// cloud layer reference
	CloudLayer & m_clouds;
	/// sun light reference
	proLight & m_sun;
};
	
Var SkyController::call(const std::string & cmd, const Var & arg) {
	if(cmd=="cloudsVisible") {
		if(arg.size()) m_clouds.visible(arg[0].boolean());
		return m_clouds.visible();
	}
	if(cmd=="cloudVelocity") {
		if(arg.size()>1) m_clouds.velocity(vec2f(arg[0].number(), arg[1].number()));
		else if(arg[0].size()>1) m_clouds.velocity(vec2f(arg[0][0].number(), arg[0][1].number()));
		return Var().append(m_clouds.velocity()[0]).append(m_clouds.velocity()[1]);
	}
	else if(cmd=="sunPosition") {
		if(arg.size()>1) {
			m_sky.sunPosition(arg[0].number(), arg[1].number()); 
			m_sky.update();
			m_sun.pos(m_sky.sunVector());
		}
		else if(arg[0].size()>1) {
			m_sky.sunPosition(arg[0][0].number(), arg[0][1].number());
			m_sky.update();
			m_sun.pos(m_sky.sunVector());
		}
		return Var().append(m_sky.sunHeading()).append(m_sky.sunPitch());
	}
	else if(cmd=="turbidity") {
		if(arg.size()) {
			m_sky.turbidity(arg[0].number());
			m_sky.update();
		}
		return m_sky.turbidity();
	}
	return Var::null;
}

//--- functions ----------------------------------------------------

void drawPlane(float sz, float z, const vec4f & color, float texSc=1.0f, unsigned int texId=0) {
	if(texId) {
		texSc*=sz;
		glEnable (GL_TEXTURE_2D );
		glBindTexture(GL_TEXTURE_2D, texId);
	}
    glBegin(GL_TRIANGLES);
    glColor4fv(&color[0]);
    glNormal3f(0,0,1);
	glTexCoord2f(-texSc, -texSc);
    glVertex3f(-sz,-sz,z);
	glTexCoord2f(texSc, -texSc);
    glVertex3f(sz,-sz,z);
	glTexCoord2f(texSc, texSc);
    glVertex3f(sz,sz,z);
	glTexCoord2f(-texSc, -texSc);
    glVertex3f(-sz,-sz,z);
	glTexCoord2f(texSc, texSc);
    glVertex3f(sz,sz,z);
	glTexCoord2f(-texSc, texSc);
    glVertex3f(-sz,sz,z);
    glEnd();
	if(texId) glDisable (GL_TEXTURE_2D );
}

void updateCamera(vec6f & pos, const vec6f & vel, double deltaT, bool doRotation) {
	const float velTranslMax=2.5f;
	const float velRotMax=90.0f;
	static vec2f mousePrev(vel[H], vel[P]);
	float cosH = static_cast<float>(dcos(pos[H]));
	float sinH = static_cast<float>(dsin(pos[H]));
	pos[X] += static_cast<float>(deltaT) * (vel[X]*cosH-vel[Y]*sinH) * velTranslMax;
	pos[Y] += static_cast<float>(deltaT) * (vel[X]*sinH+vel[Y]*cosH) * velTranslMax;
	pos[Z] += static_cast<float>(deltaT) * vel[Z] * velTranslMax;
	if(doRotation) {
		pos[H] -= (vel[H]-mousePrev[X]) * velRotMax;
		pos[P] += (vel[P]-mousePrev[Y]) * velRotMax;
	}
	// correct angle overflow:
	if (pos[H] >= 360.0f) pos[H]-=360.0f;
	else if (pos[H] <0.0f) pos[H]+=360.0f;
	if(pos[P]>90.0f) pos[P]=90.0f;
	else if(pos[P]<-90.0f) pos[P]=-90.0f;
	mousePrev.set(vel[H], vel[P]);
}

//--- main ---------------------------------------------------------
int main( int argc, char **argv ) {
	cmdLine::author     ("Gerald Franz, www.viremo.de");
	cmdLine::version    ("0.1.2");
	cmdLine::date       ("2009-08-13");
	cmdLine::shortDescr ("A protea-based scene and model viewer.");
	cmdLine::usage      ("[-i(niFile.lua)] [-jN(input from joystick n)] [-lDeviceName (input from local device)] [-x(window width)] [-y(window height)] [-f(ullscreen)] [-v(frustum vertical shift)] [scene]");
	cmdLine::interpret(argc, argv);	

	dout("loading startup script...");
	VMCallable ini;
	ini.load(cmdLine::opt('i') ? cmdLine::optArg('i') : cmdLine::dir()+"proteaViewer.lua");
	ini.eval();
	if(ini.error().size()) cerr << ini.error() << endl;
	dout(" done.\n");
    	
	// initialize devices:
	dout("initializing devices...");
	const int winSizeX = cmdLine::opt('x') ? s2ui(cmdLine::optArg('x')) : cmdLine::opt('f') ? 0 : 640;
	const int winSizeY = cmdLine::opt('y') ? s2ui(cmdLine::optArg('y')) : cmdLine::opt('f') ? 0 : 480;
	DeviceWindow* pWnd = DeviceWindowGlfw::create(winSizeX,winSizeY, cmdLine::opt('f'), true);
	if(!pWnd) return 1;
	pWnd->clearColor(0.3f, 0.3f, 0.5f);
	pWnd->title("protea viewer");
	pWnd->cursorVisible(true);
	DeviceInput * pInput = cmdLine::opt('l') ? DeviceLocal::create(cmdLine::optArg('l')) : 0;
	if(!pInput&&cmdLine::opt('j')) pInput = DeviceJoystickGlfw::create(s2ui(cmdLine::optArg('j')));
	if(!pInput) pInput = DeviceInput::instance(0u);
	assert(pInput!=0);
	TimerGlfw timer;
	dout(" done.\n");

	// initialize file format codecs:
	dout("initializing codecs...");
	ModelMgr & modelMgr = ModelMgr::singleton();
	modelMgr.loaderRegister(ioWrl::load,"wrl");
	modelMgr.loaderRegister(io3ds::load,"3ds");
	modelMgr.loaderRegister(ioObj::load,"obj");
	TextureMgr & textureMgr = TextureMgr::singleton();
	textureMgr.loaderRegister(ioPng::load,"png");
	textureMgr.loaderRegister(ioJpg::load,"jpg");
	dout(" done.\n");

	// initialize 3D visualization:
	dout("initializing 3D visualization...");
	proNode::renderer(new RendererGL);
	proCamera camera;
	float frsShiftV = cmdLine::opt('v') ? s2f(cmdLine::optArg('v')) : 0.0f;
	camera.dim().set(-1.0f, 1.0f, -.75f+frsShiftV, .75f+frsShiftV, 1.0f,1000.0f);
	camera.pos().set(0.0f,-3.0f,1.6f, 0.0f,0.0f,0.0f);
	camera.time(timer.now());
	camera.flags()=FLAG_LIGHT|FLAG_RENDER|FLAG_SHADOW;
	proScene scene("Scene");
	scene.enable(FLAG_SHADOW);
	SkyDome* pSky = new SkyDome(textureMgr.getTextureId(cmdLine::dir()+"resource/circle64.tga", false, false));
	pSky->sunPosition(30.0f, 25.0f);
	pSky->update();
	CloudLayer* pClouds= new CloudLayer(textureMgr.getTextureId(cmdLine::dir()+"resource/clouds2.tga", true, true));
	pClouds->initGraphics();
	proLight* pSun = dynamic_cast<proLight*>(scene.append(new proLight(pSky->sunVector()), false));
	SkyController* pSkyCtrl = new SkyController(*pSky, *pClouds, *pSun);
	scene.initGraphics();
	unsigned int groundTexId=textureMgr.getTextureId(cmdLine::dir()+"resource/grass_soil.jpg");
	dout(" done.\n");

	// initialize GUI:
	dout("initializing GUI...");
	Var guiIni = ini.get("GUI");
	CanvasGui* pGui = new CanvasGui(*pWnd,*pInput, guiIni);
	assert(pWnd->font()!=0);
	if((pInput->type()==DeviceWindow::s_type)
		||(dynamic_cast<DeviceVirtual*>(pInput)&&(dynamic_cast<DeviceVirtual*>(pInput)->source().type()==DeviceWindow::s_type)))
		pGui->inputByPointer(true);
	else pGui->inputByPointer(false);
	dout(pGui->inputByPointer() ? " pointer input" : " DPad input");
	dout(" loading screens...");
	pGui->append(guiIni["screen"]);
	Element2D* pInfoLeft = pGui->find("infoLeft");
	Element2D* pInfoRight = pGui->find("infoRight");
	ElementString* pInputText = dynamic_cast<ElementString*>(pGui->find("inputText"));
	assert( (pInfoLeft !=0) && (pInfoRight!=0) && (pInputText!=0) );	
	dout(" done.\n");

	// initialize application:
	dout("initializing virtual machine...");	
	VMCallable vm;
	Application app(scene, *pSun);
	vm.bind("app",app);
	vm.bind("Sky", *pSkyCtrl);
	vm.bind("Gui", *pGui);
	dout(" done.\n");
	dout("loading scene...");	
	if(cmdLine::nArg()) app.load(io::unifyPath(cmdLine::arg(0)));
	dout(" done.\n");

	double tFps=camera.time();
	unsigned int fps=0;
	const double infoTimeSpan=5.0; // after this time span the infotext is hidden
	string infoLeft="application started.";
	double tLastInfo=camera.time();
    
	// main loop:
	dout("entering main loop...\n");
	while(pWnd->open()) {
		// update timing:
		camera.time(timer.update());
		++fps;
		if(timer.now()>=tLastInfo+infoTimeSpan) 
			infoLeft=f2s(camera.pos()[X],2)+" "+f2s(camera.pos()[Y],2)+" "+f2s(camera.pos()[Z],2)+" "+f2s(camera.pos()[H],2)+" "+f2s(camera.pos()[P],2)+" "+f2s(camera.pos()[R],2);
		if(timer.now()>=tFps+1.0)  {
			string infoRight=i2s(fps)+" fps";
			pInfoRight->text(infoRight);			
			fps=0;
			tFps=timer.now();
		}
		// input handling:
		string cmd;
		string textInput(pWnd->textInput());
		if(pWnd->textComplete() && textInput.size())
			cmd += textInput+'\n';
		
		if(!pInput->button(1))
			if(pGui->update(timer.deltaT())==Widget::STATE_TRIGGERED)
				cmd+=pGui->value()+'\n';

		if(pInput->buttonDown(1)) {
			pWnd->cursorRestricted(false);
			pWnd->cursorVisible(false);
		}
		else if(pInput->buttonUp(1)) {
			pWnd->cursorRestricted(true);
			pWnd->cursorVisible(true);
		}
		
		if(pInput->buttonDown(8))
			app.drawGroundPlane(!app.drawGroundPlane());
		if(pInput->buttonDown(9))
			app.drawShadow(!app.drawShadow());
		if(pInput->buttonDown(10))
			app.drawWireFrame(!app.drawWireFrame());
		
		// update application:
		if(cmd.size()) {
			cout << cmd << flush;
			vm.eval(cmd);
			if(vm.error().size()) cerr << vm.error() << endl;
		}
		app.update(timer.deltaT());
		string msg = app.message();
		if(msg.size()) {
			infoLeft=msg;
			tLastInfo=timer.now();
		}

		// update motion model:
		vec6f cameraVel;
		pInput->axes(&cameraVel[0]);
		updateCamera(camera.pos(), cameraVel, timer.deltaT(), pInput->button(1));
		
		// draw scene:
		glPushMatrix();
		if(app.drawShadow()) camera.flags()|=FLAG_SHADOW;
		else camera.flags()&=~FLAG_SHADOW;
		camera.update();

		pSky->draw(camera);
		pClouds->draw(camera);
		
		if(app.drawWireFrame()) camera.flags()|=FLAG_WIREFRAME;
		else camera.flags()&=~FLAG_WIREFRAME;
		if(app.drawGroundPlane())
			drawPlane(200.0f,-0.01f,vec4f(1.0f,1.0f,1.0f,1.0f), 0.33f, groundTexId);
		scene.draw(camera);

		glPopMatrix();
		
		// update overlay:
		pInfoLeft->text(infoLeft);
		textInput = "> "+ textInput;
		if(fmod(timer.now(), 1.0)>0.5f) textInput+='_';
		pInputText->text(textInput);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		pGui->draw();
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		DeviceInput::updateAll(timer.deltaT());
		timer.sleep(0.01);
	}
	dout("closing down...");
	delete pGui;
	delete pSkyCtrl;
	delete pSky;
	delete pClouds;
	delete pWnd;
	dout(" done.\n");
	return 0;
}
