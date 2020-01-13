#ifndef _PRO_DEVICE_H
#define _PRO_DEVICE_H

/** @file proDevice.h
 
 \brief  interfaces for accessing input and output devices
 \version 1.3.0 2009-04-06

License notice (zlib license):

 (c) 2008-2009 by Gerald Franz, www.viremo.de

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <vector>
#include <string>
#include <map>
#include "proResource.h"

//--- class DeviceInput --------------------------------------------
/// abstract base class for input devices
class DeviceInput {
public:
	/// default constructor
	DeviceInput() : m_id(s_counter++), m_name("unknown"), m_nAxes(0), m_nButtons(0), m_axes(0), m_buttonCurr(0), m_buttonPrev(0) { sv_instance.push_back(this); }
	/// destructor
	virtual ~DeviceInput()  { if(m_nAxes) delete[] m_axes;  sv_instance[m_id]=0;}
	/// returns pointer to input device having the passed ID, if existing, or 0
	static DeviceInput* instance(unsigned int id) { return id>=sv_instance.size() ? 0 : sv_instance[id]; }
	/// returns pointer to input device having the passed name, if existing, or 0
	static DeviceInput* instance(const std::string & name);

	// device property access
	/// returns ID
	unsigned int id() const { return m_id; }
	/// returns instance specific name
	const std::string & name() const { return m_name; }
	/// returns class specific type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
	/// returns number of axes
	unsigned int nAxes() const { return m_nAxes; }
	/// returns number of buttons
	unsigned int nButtons() const { return m_nButtons; }

	// input data access
	/// updates device state, abstract
	virtual int update(double deltaT=0.0)=0;
	/// updates all devices at once
	static int updateAll(double deltaT=0.0);
	/// fills a float array of size nAxes() with the values of the input axes
	void axes(float *a) const;
	/// returns current value of input axis n
	virtual float axis(unsigned int n) const { return n<m_nAxes ? m_axes[n] : 0.0f; }
	/// returns current state of button or key n
	virtual bool button(unsigned int n) const { return (m_buttonCurr&(1<<(n%32)))>0; }
	/// returns whether a button down event has occured during the last frame
	virtual bool buttonDown(unsigned int n) const { return (m_buttonCurr&(1<<(n%32))) && !(m_buttonPrev&(1<<(n%32))); }
	/// returns whether a button up event has occured during the last frame
	virtual bool buttonUp(unsigned int n) const { return !(m_buttonCurr&(1<<(n%32))) && (m_buttonPrev&(1<<(n%32))); }
protected:
	/// stores ID
	unsigned int m_id;
	/// stores name
	std::string m_name;
	/// stores number of axes
	unsigned int m_nAxes;
	/// stores number of buttons
	unsigned int m_nButtons;	
	/// stores axes values
	float* m_axes;
	/// stores current input button state
	unsigned int m_buttonCurr;
	/// stores previous input button state
	unsigned int m_buttonPrev;	
private:
	/// stores pointers to device instances
	static std::vector<DeviceInput*> sv_instance;
	/// device counter, for ID generation
	static unsigned int s_counter;
};

//--- class DeviceVirtual ------------------------------------------
/// a virtual device allowing for flexible remapping
class DeviceVirtual : public DeviceInput {
public:
	/// constructor
	DeviceVirtual(DeviceInput & source, const std::string & name) : DeviceInput(), m_src(source), 
		m_now(0.0), m_autoRepeatDelay(0.4f), m_autoRepeatInterval(0.1f) { m_name = name; }
	/// returns class specific type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
	/// updates device state
	virtual int update(double deltaT=0.0);
	/// returns current state of button or key n
	virtual bool button(unsigned int n) const {
		return n>=mv_button.size() ? false : mv_button[n]; }
	/// returns whether a button down event has occured during the last frame
	virtual bool buttonDown(unsigned int n) const {
		return n>=mv_button.size() ? false : (mv_button[n]&&!mv_buttonPrev[n]); }
	/// returns whether a button up event has occured during the last frame
	virtual bool buttonUp(unsigned int n) const {
		return n>=mv_button.size() ? false : (!mv_button[n]&&mv_buttonPrev[n]); }
	/// allows read access to the device's source
	const DeviceInput & source() const { return m_src; }
	
	// remapping 
	/// constant indicating an unused axis or button
	static const unsigned char NONE=255;
	/// maps input axis to output axis
	bool mapAxis(unsigned char output, unsigned char input, float scale = 1.0f, float shift=0.0f);
	/// maps input button to output button
	bool mapButton(unsigned char output, unsigned char input, float scale = 1.0f, float shift=0.0f, bool autoRepeat = false);
	/// maps one or two buttons to an axis
	bool mapButtonToAxis(unsigned char output, unsigned char inputHi, unsigned char inputLo=NONE, float scale = 1.0f, float shift=0.0f );
	/// maps an input axis to one or two output buttons
	bool mapAxisToButton(unsigned char axis, unsigned char buttonHi, unsigned char buttonLo=NONE, float scale = 1.0f, float shift=0.0f, bool autoRepeat = false );

	/// sets auto repeat delay
	void autoRepeatDelay(float d) { m_autoRepeatDelay=d; }
	/// returns auto repeat delay
	float autoRepeatDelay() const { return m_autoRepeatDelay; }
	/// sets auto repeat interval
	void autoRepeatInterval(float d) { m_autoRepeatInterval=d; }
	/// returns auto repeat interval
	float autoRepeatInterval() const { return m_autoRepeatInterval; }
protected:
	/// reference to source device
	DeviceInput & m_src;
	/// internal class storing a single mapping rule
	class Mapping {
	public:
		/// default constructor generating an empty mapping
		Mapping() : srcAxis(NONE), srcButtonLo(NONE), srcButtonHi(NONE) { }
		/// checks mapping validity
		bool isValid() const { return srcAxis!=NONE || srcButtonHi!=NONE; }
		/// stores source axis ID
		unsigned char srcAxis; 
		/// stores targer axis ID
		unsigned char tgtAxis;
		/// stores low source value button ID
		unsigned char srcButtonLo; 
		/// stores high source value button ID
		unsigned char srcButtonHi; 
		/// stores low target value button ID
		unsigned char tgtButtonLo; 
		/// stores high target value button ID
		unsigned char tgtButtonHi;
		/// stores output scale factor
		float scale;
		/// stores output shift
		float shift;
	};
	/// vector containing mapping rules
	std::vector<Mapping> mv_mapping;
	/// vector containing current button values
	std::vector<bool> mv_button;
	/// vector containing button values of previous frame
	std::vector<bool> mv_buttonPrev;
	/// vector defining whether auto repeat is enabled for this button and storing buttondown timestamp
	std::vector<double> mv_autoRepeat;
	/// current time stamp
	double m_now;
	/// global auto repeat delay
	float m_autoRepeatDelay;
	/// global auto repeat interval
	float m_autoRepeatInterval;
};

//--- class ListenerResize -----------------------------------------
/// abstract interface for classes that want to get notified by resize events
class ListenerResize {
public:
	/// destructor
	virtual ~ListenerResize() { }
	/// method called by a resize event providing the new dimensions and indicating if a reload of graphical resources is required
	virtual void resize(float width, float height)=0;
};

//--- class DeviceWindow  ------------------------------------------
/// abstract base class for window devices
class DeviceWindow : public DeviceInput {
public:
	/// returns a previously created window instance
	/** \param id (optional, default=0) window ID. Currently ignored, windows are singletons
	\return pointer to window device or 0 in case the ID is invalid */
	static DeviceWindow * instance(unsigned int id=0) { return sp_instance; }
	/// destructor, closes window and cleans up resources
	virtual ~DeviceWindow();	
	/// returns class specific type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
	/// initializes graphics and resources
	virtual void initGraphics()=0;
	/// updates device state
	virtual int update(double deltaT=0.0);
	
	// Window Management API
	/// returns width
	unsigned int width() const { return m_w; }
	/// returns height
	unsigned int height() const { return m_h; }
	/// sets clear color
	virtual void clearColor(float r, float g, float b)=0;
	/// minimizes window
	virtual void minimize()=0;
	/// restores window
	virtual void restore()=0;
	/// returns true in case the window is fullscreen
	virtual bool fullScreen() const { return m_fullscreen; }
	/// switches fullscreen mode on/off at runtime
	virtual void fullScreen(bool fullscreen)=0;
	/// registers a listener for resize events
	void regListenerResize(ListenerResize * listener) { mv_listenerResize.push_back(listener); }
	/// unregisters a listener for resize events
	bool unregListenerResize(ListenerResize * listener);
	/// sets window title
	virtual void title(const std::string & s)=0;
	/// returns true in case the window is open
	bool open() const { return m_open; }
	/// closes window
	void close() { m_open = false; }
	/// sets mouse cursor visibility
	virtual void cursorVisible(bool visible) { }
	/// returns mouse cursor visibility
	bool cursorVisible() const { return m_cursorVisible; }
	/// returns true if cursor is restricted to window area
	bool cursorRestricted() const { return m_cursorRestricted; }
	/// (un-)restricts cursor to window area
	void cursorRestricted(bool yesno) { m_cursorRestricted=yesno; }
	/// resets cursor position to the window's center
	virtual void cursorReset() { }

	/// returns current text input
	const char* textInput() const { return m_textInput; }
	/// returns true in case the text input has been completed, e.g., by  pressing RETURN, interface definition
	bool textComplete() const { return m_textInputComplete; }
	/// sets current text input, pass an empty string or 0 to reset
	void textInput(const char * text);
	/// symbolic names for some special keys
	enum {
		KEY_ESC = 27,
		KEY_ENTER = 28,
		KEY_TAB = 29,
	};

	// Timer API
	/// returns current timestamp
	/** constant within one frame */
	double now() const { return m_now; }
	/// time difference to previous frame
	double deltaT() const { return m_deltaT; }
	/// updates current frame time without setting deltaT, to avoid jerks after blocking operations
	virtual void updateTime() { }
	
	// Resource Management API
	/// flags for createTexture()
	enum {
		TEXFLAG_8BIT_ALPHA = 1<<0,
		TEXFLAG_REPEAT = 1<<2,
		TEXFLAG_CLAMP_TO_EDGE = 1<<3,
	};
	/// uploads an image to texture memory und returns texture ID
	virtual unsigned int createTexture( const Image & img, const std::string & name, unsigned int texFlags = TEXFLAG_8BIT_ALPHA )=0;
	/// returns the texture ID associated with this name, or 0 in case no texture is associated
	unsigned int textureId(const std::string & name) const;
	/// returns properties of a previously loaded texture by name
	bool textureProperties(const std::string & name, unsigned int & id, unsigned int & width, unsigned int & height,  unsigned int & depth) const;
	/// uploads a font image to texture memory und returns font
	virtual Font* createFont(const Image & img, const std::string & name="")=0;
	/// returns the font associated with this name, or a default font (if available) in case no font is associated
	Font* font(const std::string & name="") const;
	/// defines a color accessible via its name
	void createColor(float r, float g, float b, float a, const std::string & name) {
		mm_color[name]=Color(r,g,b,a); }
	/// returns the color associated with this name, or black in case no color is associated
	const Color & color(const std::string & name);
protected:
	/// constructor
	DeviceWindow(unsigned int width, unsigned int height, bool fullScreen) : 
		DeviceInput(), m_w(width), m_h(height), m_wOriginal(width), m_hOriginal(height), m_open(true), 
		m_fullscreen(fullScreen), m_cursorVisible(true), m_cursorRestricted(false), m_title("protea window"),
		m_textInputSize(0), m_textInputComplete(false), m_deltaT(0.0), mp_fontDefault(0) { 
		m_clearColor[0] = m_clearColor[1] = m_clearColor[2] = 0.0f; }
	
	/// stores width and height
	unsigned int m_w, m_h;
	/// stores original width and height before resizing or switching to fullscreen
	unsigned int m_wOriginal, m_hOriginal;
	/// stores whether window is open
	bool m_open;
	/// stores whether window is fullscreen
	bool m_fullscreen;
	/// stores whether mouse pointer is visible
	bool m_cursorVisible;
	/// stores whether mouse pointer is restricted to window area
	bool m_cursorRestricted;
	/// stores window title
	std::string m_title;
	/// stores clear color
	float m_clearColor[3];

	/// pointer to singleton instance
	static DeviceWindow * sp_instance;
	
	///  stores current text input size
	unsigned int m_textInputSize;
	///  indicates that text input has been completed
	bool m_textInputComplete;
	/// maximum text input length
	const static unsigned int sc_textInputMax = 256;
	/// stores text input
	char m_textInput[sc_textInputMax];
	
	/// current timestamp of this frame
	double m_now;
	/// time difference to previous frame
	double m_deltaT;
	
	/// internal structure holding basic texture properties
	struct TexProperty {
		/// constructor
		TexProperty(unsigned int id, unsigned int w, unsigned int h, unsigned int d) : texId(id), width(w), height(h), depth(d) { }
		/// texture ID
		unsigned int texId;
		/// original image width in pixels
		unsigned int width;
		/// original image height in pixels
		unsigned int height;
		/// image depth in bytes
		unsigned int depth;
	};
	/// associative array between texture names and texture properties
	std::map<std::string, TexProperty*> mm_tex;
	/// associative array between font names and Font objects
	std::map<std::string, Font*> mm_font;
	/// pointer to default font
	Font* mp_fontDefault;
	/// associative array between color names and Color objects
	std::map<std::string, Color> mm_color;
	/// window resize listeners
	std::vector<ListenerResize*> mv_listenerResize;
};

#endif //_PRO_DEVICE_H
