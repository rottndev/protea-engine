#include "proDevice.h"
#include <iostream>

using namespace std;

//--- class DeviceInput --------------------------------------------

const char* DeviceInput::s_type = "generic";

unsigned int DeviceInput::s_counter = 0;
vector<DeviceInput*> DeviceInput::sv_instance;

void DeviceInput::axes(float *a) const {
	memcpy(a,m_axes,m_nAxes*sizeof(float));
}

DeviceInput* DeviceInput::instance(const std::string & name) { 
	for(vector<DeviceInput*>::iterator it=sv_instance.begin(); it!=sv_instance.end(); ++it)
		if((*it)->name()==name) return *it;
	return 0; 
}

int DeviceInput::updateAll(double deltaT) {
	int ret=0;
	for(vector<DeviceInput*>::iterator it=sv_instance.begin(); it!=sv_instance.end(); ++it)
		ret+=(*it)->update(deltaT);
	return ret;
}

//--- class DeviceVirtual ------------------------------------------

const char* DeviceVirtual::s_type = "virtual";

int DeviceVirtual::update(double deltaT) {
	m_now+=deltaT;
	mv_buttonPrev = mv_button;
	memset(m_axes,0,sizeof(float)*m_nAxes);
	for(vector<Mapping>::iterator it = mv_mapping.begin(); it!=mv_mapping.end(); ++it) {
		if(!it->isValid()) continue;
		float value = it->srcAxis!= NONE ? m_src.axis(it->srcAxis) : ((it->srcButtonLo!=NONE) && m_src.button(it->srcButtonLo)) ? -1.0f : m_src.button(it->srcButtonHi);
		value = value*it->scale + it->shift;
		if(it->tgtAxis!=NONE) m_axes[it->tgtAxis] = value;
		else if(it->tgtButtonLo!=NONE) {
			mv_button[it->tgtButtonLo]= value<=-1.0f ? true : false;
			if(mv_button[it->tgtButtonLo]&& (mv_autoRepeat[it->tgtButtonLo]>=0.0)) {
				if(!mv_buttonPrev[it->tgtButtonLo] ) 
					mv_autoRepeat[it->tgtButtonLo]=m_now+m_autoRepeatDelay; // store autorepeat start time
				else if(m_now>=mv_autoRepeat[it->tgtButtonLo]) { // trigger autorepeat
					mv_buttonPrev[it->tgtButtonLo]=false;
					mv_autoRepeat[it->tgtButtonLo]=m_now+m_autoRepeatInterval;
				}
			}

			mv_button[it->tgtButtonHi]= value>=1.0f ? true : false;
			if(mv_button[it->tgtButtonHi]&& (mv_autoRepeat[it->tgtButtonHi]>=0.0)) {
				if(!mv_buttonPrev[it->tgtButtonHi] ) 
					mv_autoRepeat[it->tgtButtonHi]=m_now+m_autoRepeatDelay; // store autorepeat start time
				else if(m_now>=mv_autoRepeat[it->tgtButtonHi]) { // trigger autorepeat
					mv_buttonPrev[it->tgtButtonHi]=false;
					mv_autoRepeat[it->tgtButtonHi]=m_now+m_autoRepeatInterval;
				}
			}
		}
		else {
			mv_button[it->tgtButtonHi]= value>=1.0f ? true : false;
			if(mv_button[it->tgtButtonHi]&& (mv_autoRepeat[it->tgtButtonHi]>=0.0)) {
				if(!mv_buttonPrev[it->tgtButtonHi] ) 
					mv_autoRepeat[it->tgtButtonHi]=m_now+m_autoRepeatDelay; // store autorepeat start time
				else if(m_now>=mv_autoRepeat[it->tgtButtonHi]) { // trigger autorepeat
					mv_buttonPrev[it->tgtButtonHi]=false;
					mv_autoRepeat[it->tgtButtonHi]=m_now+m_autoRepeatInterval;
				}
			}
		}
	}
	return 0;
}

bool DeviceVirtual::mapAxis(unsigned char output, unsigned char input, float scale, float shift) {
	if(input>=m_src.nAxes()) return false;
	if(output>=m_nAxes) {
		delete [] m_axes;
		m_nAxes = output+1;
		m_axes = new float[m_nAxes];
		memset(m_axes,0,sizeof(float)*m_nAxes);
	}
	vector<Mapping>::iterator it;
	for(it = mv_mapping.begin(); it!=mv_mapping.end(); ++it)
		if(it->tgtAxis==output) break;
	if(it==mv_mapping.end()) {
		mv_mapping.push_back(Mapping());
		it=mv_mapping.end()-1;
	}
	it->srcButtonLo = it->srcButtonHi = it->tgtButtonLo = it->tgtButtonHi = NONE;
	it->srcAxis = input;
	it->tgtAxis = output;
	it->scale = scale;
	it->shift = shift;
	return true;
}

bool DeviceVirtual::mapButton(unsigned char output, unsigned char input, float scale, float shift, bool autoRepeat) {
	if(input>=m_src.nButtons()) return false;
	while(output>=mv_button.size()) {
		mv_button.push_back(false);
		mv_buttonPrev.push_back(false);
		mv_autoRepeat.push_back(-1.0);
		++m_nButtons;
	}
	if(autoRepeat) mv_autoRepeat[output]=0.0;
	vector<Mapping>::iterator it;
	for(it = mv_mapping.begin(); it!=mv_mapping.end(); ++it)
		if((it->tgtButtonHi==output)||(it->tgtButtonLo==output)) break;
	if(it==mv_mapping.end()) {
		mv_mapping.push_back(Mapping());
		it=mv_mapping.end()-1;
	}
	it->srcAxis = it->tgtAxis = it->srcButtonLo = it->tgtButtonLo = NONE;
	it->srcButtonHi = input;
	it->tgtButtonHi = output;
	it->scale = scale;
	it->shift = shift;
	return true;
}

bool DeviceVirtual::mapButtonToAxis(unsigned char output, unsigned char inputHi, unsigned char inputLo, float scale, float shift) {
	if(inputHi>=m_src.nButtons()) return false;
	if((inputLo!=NONE)&&(inputLo>=m_src.nButtons())) return false;
	if(output>=m_nAxes) {
		delete [] m_axes;
		m_nAxes = output+1;
		m_axes = new float[m_nAxes];
		memset(m_axes,0,sizeof(float)*m_nAxes);
	}
	vector<Mapping>::iterator it;
	for(it = mv_mapping.begin(); it!=mv_mapping.end(); ++it)
		if(it->tgtAxis==output) break;
	if(it==mv_mapping.end()) {
		mv_mapping.push_back(Mapping());
		it=mv_mapping.end()-1;
	}
	it->srcAxis = it->tgtButtonLo = it->tgtButtonHi = NONE;
	it->tgtAxis = output;
	it->srcButtonLo = inputLo;
	it->srcButtonHi = inputHi;
	it->scale = scale;
	it->shift = shift;
	return true;
}

bool DeviceVirtual::mapAxisToButton(unsigned char axis, unsigned char buttonHi, unsigned char buttonLo, float scale, float shift, bool autoRepeat) {
	if(axis>=m_src.nAxes()) return false;
	while(buttonHi>=mv_button.size()) {
		mv_button.push_back(false);
		mv_buttonPrev.push_back(false);
		mv_autoRepeat.push_back(-1.0);
		++m_nButtons;
	}
	if(autoRepeat) mv_autoRepeat[buttonHi]=0.0;
	if(buttonLo!=NONE) while(buttonLo>=mv_button.size()) {
		mv_button.push_back(false);
		mv_buttonPrev.push_back(false);
		mv_autoRepeat.push_back(-1.0);
		++m_nButtons;
	}
	if(autoRepeat) mv_autoRepeat[buttonLo]=0.0;
	vector<Mapping>::iterator it;
	for(it = mv_mapping.begin(); it!=mv_mapping.end(); ++it)
		if((it->tgtButtonHi==buttonHi)||(it->tgtButtonLo==buttonHi)
			||(it->tgtButtonHi==buttonLo)||(it->tgtButtonLo==buttonLo)) break;
	if(it==mv_mapping.end()) {
		mv_mapping.push_back(Mapping());
		it=mv_mapping.end()-1;
	}
	it->srcAxis = axis;
	it->tgtAxis = it->srcButtonLo = it->srcButtonHi = NONE;
	it->tgtButtonHi = buttonHi;
	it->tgtButtonLo = buttonLo;
	it->scale = scale;
	it->shift = shift;
	return true;
}

//--- class DeviceWindow ---------------------------------------

DeviceWindow * DeviceWindow::sp_instance=0;
const char* DeviceWindow::s_type = "window";


DeviceWindow::~DeviceWindow() {
	for(map<string,Font*>::iterator it = mm_font.begin(); it!= mm_font.end(); ++it)
		if(it->second!=mp_fontDefault) delete it->second;
	delete mp_fontDefault;
	for(map<string,TexProperty*>::iterator it = mm_tex.begin(); it!= mm_tex.end(); ++it)
		delete it->second;
}

int DeviceWindow::update(double deltaT) {
	if(m_textInputComplete&&m_textInputSize) {
		m_textInputSize=0; 
		m_textInputComplete = false; 
		m_textInput[0]=0;
	}
	return 0;
}

void DeviceWindow::textInput(const char * text) {
	if(!text||!strlen(text)) {
		m_textInputSize=0; 
		m_textInput[0]=0; 
	}
	else {
		m_textInputSize = min(strlen(text),sc_textInputMax-1);
		strncpy(m_textInput, text, m_textInputSize);
		m_textInput[m_textInputSize]=0;
	}
	m_textInputComplete=false; 
}

bool DeviceWindow::unregListenerResize(ListenerResize * listener) {
	for(vector<ListenerResize*>::iterator it = mv_listenerResize.begin(); it!=mv_listenerResize.end(); ++it) if((*it)==listener) {
		mv_listenerResize.erase(it);
		return true;
	}
	return false;
}

unsigned int DeviceWindow::textureId(const std::string & name) const {
	map<string,TexProperty*>::const_iterator it = mm_tex.find(name);
	return it==mm_tex.end() ? 0 : it->second->texId;
}

bool DeviceWindow::textureProperties(const std::string & name, unsigned int & id, unsigned int & width, unsigned int & height,  unsigned int & depth) const {
	map<string,TexProperty*>::const_iterator it = mm_tex.find(name);
	if(it==mm_tex.end()) return false;
	id = it->second->texId;
	width = it->second->width;
	height = it->second->height;
	depth = it->second->depth;
	return true;
}

Font* DeviceWindow::font(const std::string & name) const {
	if(!name.size()) return mp_fontDefault;
	map<string,Font*>::const_iterator it = mm_font.find(name);
	return it!=mm_font.end() ? it->second : mp_fontDefault;
}

const Color & DeviceWindow::color(const std::string & name) {
	map<string,Color>::const_iterator it = mm_color.find(name);
	return it!=mm_color.end() ? it->second : Color::null;
}
