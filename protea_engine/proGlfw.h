#ifndef _PRO_GLFW_H
#define _PRO_GLFW_H

/** @file proGlfw.h
 
 \brief  GLFW based window and joystick device
 \version 1.6.0 2009-08-15

 License notice (zlib license):

 (c) 2007-2009 by Gerald Franz, www.viremo.de

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

#include "proDevice.h"
#include <glfw.h>

//--- class DeviceWindowGlfw  --------------------------------------
/// a singleton class wrapping GLFW window and input functions
class DeviceWindowGlfw : public DeviceWindow {
public:
	/// creates an GLFW based OpenGL window
	static DeviceWindow * create(unsigned int sizeX, unsigned int sizeY, bool fullScreen, bool stencilBuffer=false);
	/// destructor, closes OpenGL window
	virtual ~DeviceWindowGlfw();	
	
	// input data access API
	/// updates device state and swaps buffers
	virtual int update(double deltaT=0.0);
	/// returns current state of button or key n
	virtual bool button(unsigned int n) const;

	/// sets clear color
	virtual void clearColor(float r, float g, float b);
	/// minimizes window
	virtual void minimize() { glfwIconifyWindow(); }
	/// restores window
	virtual void restore() { glfwRestoreWindow(); }
	/// returns true in case the window is fullscreen
	virtual bool fullScreen() const { return m_fullscreen; }
	/// switches fullscreen mode on/off at runtime
	virtual void fullScreen(bool fullscreen);
	/// sets window title
	virtual void title(const std::string & s) { m_title=s; glfwSetWindowTitle(s.c_str()); }
	/// sets mouse cursor visibility
	virtual void cursorVisible(bool visible);
	/// returns mouse cursor visibility
	bool cursorVisible() { return m_cursorVisible; }
	/// resets cursor position to the window's center
	virtual void cursorReset() { glfwSetMousePos( m_w/2, m_h/2 ); }

	/// updates current frame time without setting deltaT, to avoid jerks after blocking operations
	virtual void updateTime() { m_now = glfwGetTime(); }

	/// uploads an image to texture memory und returns texture ID
	virtual unsigned int createTexture( const Image & img, const std::string & name, unsigned int texFlags = TEXFLAG_8BIT_ALPHA );
	/// uploads a font image to texture memory und returns font
	virtual Font* createFont(const Image & img, const std::string & name="");
protected:
	/// constructor
	DeviceWindowGlfw(unsigned int width, unsigned int height, bool fullScreen, bool stencilBuffer);
	/// initializes graphics and resources
	virtual void initGraphics();
	/// window close callback
	static int GLFWCALL cbClose() {
		if(sp_instance) sp_instance->close(); return true; }
	/// window resize callback
	static void  GLFWCALL cbResize( int width, int height ) {
		if(sp_instance) dynamic_cast<DeviceWindowGlfw*>(sp_instance)->resize(width, height); }
	/// notifies window instance about a resize event
	void resize(int width, int height);
	/// static text input callback
	static void GLFWCALL cbTextInput( int character, int action ) {
		if(sp_instance) dynamic_cast<DeviceWindowGlfw*>(sp_instance)->processTextInput(character, action); }
	/// instance-specific text input callback
	void processTextInput( int character, int action );
	/// static (special) key input callback
	static void GLFWCALL cbKeyInput( int key, int action ) {
		if(sp_instance) dynamic_cast<DeviceWindowGlfw*>(sp_instance)->processKeyInput(key, action); }
	/// instance specific (special) key input callback
	void processKeyInput( int key, int action );
		
	/// stores clear bits
	unsigned int m_clearBits;
};

//--- class DeviceJoystickGlfw  ------------------------------------
/// a GLFW based joystick input class
class DeviceJoystickGlfw : public DeviceInput {
public:
	/// factory method
	static DeviceInput * create(int joystickId=0);
	/// destructor
	virtual ~DeviceJoystickGlfw();
	/// returns class specific type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;

	/// updates device state
	virtual int update(double deltaT=0.0);
	/// tests whether a joystick is available
	static bool isAvailable(unsigned int joystickId=0);
protected:
	/// constructor
	DeviceJoystickGlfw(int joystickId);
	/// stores joystick id
	int m_id;
};

//--- class TimerGlfw ---------------------------------------------
///  GLFW based double precision timer class
class TimerGlfw {
public:
  /// constructor
  /** \param initTime (optional) sets the start time for the timer.  */
  TimerGlfw(double initTime = 0.0) { m_currentTime = m_lastUpdate = initTime; m_lastReset = stamp(); }
  /// sets the timer time to the given value
  void set(double time) { m_currentTime = m_lastUpdate = time; m_lastReset = stamp(); }
  /// updates the timer time according to the system clock and returns the updated time
  double update() { 
	  m_lastUpdate = m_currentTime; m_currentTime += stamp() - (m_lastReset + m_currentTime); return m_currentTime; }
  /// returns the current timer time (will always be the same between two update calls)
  double now() const { return m_currentTime; }
  /// returns the time that passed between the last two consecutive update calls
  double deltaT() const { return m_currentTime - m_lastUpdate; }

  /// returns a time stamp with double precision
  static double stamp() { return glfwGetTime(); }
  /// portable sleep function
  /** Lets the currently running process sleep for the given amount of time
   in seconds. For example a Timer::sleep(0.010) would let the process
   sleep for about 10ms and allows a general update rate of 100Hz.
   Be aware that due to operating system restrictions the finest sleep
   granularity is often only 0.01 seconds.
   \param     sec the timespan the process should sleep
   */
  static void sleep(double sec) { glfwSleep(sec); }
protected:
  /// stores current time, time of latest update call
  double m_currentTime;
  /// stores time stamp of previous update
  double m_lastUpdate;
  /// when has the time been set manually
  double m_lastReset;
};

#endif // _PRO_GLFW_H
