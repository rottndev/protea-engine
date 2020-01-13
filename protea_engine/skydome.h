// a physics based sky model

/* License (zlib license):

 (c) 2006-2009 by Gerald Franz, www.viremo.de

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

/* Acknowlegments:\n
   The day sky implementation is based on the SIGgraph '99 paper 
   "A Practical Analytic Model for Daylight" by A. J. Preetham, Peter Shirley, and Brian Smits  
*/

#include "proMath.h"
#include "proScene.h"

// --- class SkyDome -----------------------------------------------
/// draws a physics-oriented SkyDome
/** based on the SIGgraph '99 paper "A Practical Analytic Model for Daylight" by A. J. Preetham, Peter Shirley, and Brian Smits  */
class SkyDome {
public:
	/// constructor
	SkyDome(unsigned int sunTextureId, float r=20.0f, unsigned int nHor=36, unsigned int nVer=36);
	/// destructor
	virtual ~SkyDome();
	/// draws SkyDome
	virtual void draw(proCamera & camera);
	/// updates sky colors depending on sun position and turbidity
	virtual void update();

	/// calculates single sky color value at theta, phi
	vec3f color( float theta, float phi ) const;
	/// sets sun position
	void sunPosition( float heading, float pitch );
	/// returns sun's heading angle
	float sunHeading() const { return m_phiSun*RAD2DEG; }
	/// returns sun's pitch angle
	float sunPitch() const { return 90.0f-m_thetaSun*RAD2DEG; }
	/// calculates and returns sun vector
	vec4f sunVector() const;
	/// returns turbidity
	float turbidity() const { return m_turbidity; }
	/// sets turbidity, 2.0= very clear 4.0=normal, 8.0=dim
	void turbidity(float t ) { m_turbidity = t; }	
	/// sets gamma correction value
	void gamma(float g) { m_gamma=g; }
protected: 
	/// stores sun's texture id
	unsigned int m_sunId;
	/// turbidity
	float m_turbidity;    
	/// sun vertical angle from zenith
	float m_thetaSun;
	/// sun horizontal angle
	float m_phiSun;
	/// stores zenith color in xyY colorspace
	vec3f m_zenCol;
	/// stores gamma correction factor
	float m_gamma;

	/// stores vertices
	vec3f * m_vVtx;
	/// stores vertex colors
	vec3f * m_vCol;
	/// stores number of horizontal segments
	unsigned int m_nHor;
	/// stores number of vertical segments
	unsigned int m_nVer;    
};

//--- class CloudLayer ---------------------------------------------

/// draws an animated cloud layer using a texture
class CloudLayer {
public:
	/// constructor
	CloudLayer(unsigned int texId) : m_texId(texId), m_color(1.0f,1.0f,1.0f,0.33f), m_vel(0.005f, -0.007f), m_visible(true) { }
	/// destructor
	virtual ~CloudLayer() { }
	/// does graphics intialization
	virtual void initGraphics();
	/// draws object
	virtual void draw(proCamera & camera);
	/// returns true if clouds are visible
	bool visible() const { return m_visible; }
	/// controls cloud visibility
	void visible(bool yesno) { m_visible=yesno; }
	/// returns cloud color
	const vec4f & color() const { return m_color; }
	/// sets cloud color
	void color(const vec4f & c) { m_color=c; }
	/// sets cloud velocity
	void velocity(const vec2f & v) { m_vel=v; }
	/// returns cloud velocity
	const vec2f & velocity() const { return m_vel; }
protected:
	/// stores texture id
	unsigned int m_texId;
	/// stores cloud base color
	vec4f m_color;
	/// stores cloud velocity
	vec2f m_vel;
	/// stores if clouds are visible
	bool m_visible;
};
