// a physics based sky model
#include "skydome.h"

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cmath>

# ifdef _MSC_VER
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif
# include <GL/gl.h>
# include <GL/glu.h>

using namespace std;

//--- auxiliary functions ------------------------------------------
/// Perez function
static inline float perez( float A, float B, float C, float D, float E, float Theta, float Gamma ) {
    float cosGamma = cos(Gamma);
    return (1+ A * exp(B/cos(Theta)))*(1+ C * exp(D*Gamma) + E * cosGamma*cosGamma );
}

/// gamma correction function
static inline void correctGamma(vec3f &rgb, float gamma ) {
    rgb[0] = pow(rgb[0], 1.0f/gamma);
    rgb[1] = pow(rgb[1], 1.0f/gamma);
    rgb[2] = pow(rgb[2], 1.0f/gamma);
}

static vec3f xyY2rgb( const vec3f & xyY) { 
	vec3f xyz(xyY[0] * (xyY[2] / xyY[1]), xyY[2], (1.0f - xyY[0] - xyY[1])* (xyY[2]/xyY[1]));
    return vec3f(3.240479f * xyz[X] - 1.537150f * xyz[Y] - 0.498535f * xyz[Z],
        -0.969256f * xyz[X] + 1.875991f * xyz[Y] + 0.041556f * xyz[Z],
        0.055648f * xyz[X] - 0.204043f * xyz[Y] + 1.057311f * xyz[Z]);
}

/// calculates an exposure function for single float values
static inline float expose(float f, float exposure ) {
    return 1.0f-exp(-f*exposure);
}

/// feeble gradient function for night sky
static float gradient(float x, float n) {
    return (x<(1.0f/(n+1.0f))) ? 1.0f-n*x : -(1.0f/n)*x+(1.0f/n);
}

// --- class SkyDome -----------------------------------------------

SkyDome::SkyDome(unsigned int sunTextureId, float r, unsigned int nHor, unsigned int nVer) : 
	m_sunId(sunTextureId), m_turbidity(4.0f), m_thetaSun(0.0f), m_phiSun(0.0f), m_nHor(nHor), m_nVer(nVer/2+1) {
	m_gamma=1.5f;

	m_vVtx=new vec3f[m_nHor*m_nVer];
	m_vCol=new vec3f[m_nHor*m_nVer];
	for(unsigned int j=0; j<m_nVer; ++j) {
		float pitch=static_cast<float>(j)/static_cast<float>(m_nVer-1)*M_PI*0.5f;
		float z=sin(pitch);
		float cosZ=cos(pitch);
		for(unsigned int i=0; i<m_nHor; ++i) {
			float phi=2.0f*M_PI*static_cast<float>(i)/static_cast<float>(m_nHor);
			m_vVtx[i+m_nHor*j].set(cos(phi)*r*cosZ, sin(phi)*r*cosZ, z*r);
		}
	}
	update();
}

SkyDome::~SkyDome() {
    delete [] m_vVtx;
    delete [] m_vCol;
}

void SkyDome::sunPosition( float heading, float pitch ) {
    m_thetaSun = fabs(DEG2RAD*(90.0f-pitch)); 
    m_phiSun = heading*DEG2RAD;
}

void SkyDome::update() {
    float T2 = m_turbidity * m_turbidity;
    // calculate zenith color:
    float chi = (4.0f/9.0f - m_turbidity/120.0f)*(M_PI - 2.0f*m_thetaSun);
    m_zenCol[2] = (4.0453f*m_turbidity - 4.9710f)*tan(chi) - 0.2155f*m_turbidity + 2.4192f;
    if (m_zenCol[2] < 0.0f) m_zenCol[2] = -m_zenCol[2];
	float thetaS2 = m_thetaSun * m_thetaSun;
	float thetaS3 = m_thetaSun * thetaS2;
	m_zenCol[0] = ( 0.00165f * thetaS3 - 0.00375f * thetaS2 + 0.00209f * m_thetaSun + 0.00000f) * T2 +
		 (-0.02903f * thetaS3 + 0.06377f * thetaS2 - 0.03202f * m_thetaSun + 0.00394f) * m_turbidity +
		 ( 0.11693f * thetaS3 - 0.21196f * thetaS2 + 0.06052f * m_thetaSun + 0.25886f);
    m_zenCol[1] = ( 0.00275f * thetaS3 - 0.00610f * thetaS2 + 0.00317f * m_thetaSun + 0.00000f) * T2 +
		 (-0.04214f * thetaS3 + 0.08970f * thetaS2 - 0.04153f * m_thetaSun + 0.00516f) * m_turbidity +	
		 ( 0.15346f * thetaS3 - 0.26756f * thetaS2 + 0.06670f * m_thetaSun + 0.26688f);
    // calculate vertex colors:
    // TODO: make use of symmetry, calculate only half of positions
    for(unsigned int j=0; j<m_nVer; ++j) {
        float pitch=static_cast<float>(j)/static_cast<float>(m_nVer-1)*M_PI*0.5f;
        for(unsigned int i=0; i<m_nHor; ++i) {
            float theta=M_PI*0.5f-pitch;
            if(!pitch) theta=M_PI*0.5f-0.001f;
            float phi=2.0f*M_PI*static_cast<float>(i)/static_cast<float>(m_nHor);
            m_vCol[i+m_nHor*j]=color(theta,phi);
            correctGamma(m_vCol[i+m_nHor*j],m_gamma);
        }
    }
}

vec3f SkyDome::color( float theta, float phi ) const {
    const float nightBrightness=0.07f;
    if(theta<0.0f) theta=-theta;
    float f=gradient(1.0f-2.0f*theta/M_PI,3.0f);
    if(m_thetaSun>97.5f*DEG2RAD) return vec3f(nightBrightness*f,nightBrightness*f,nightBrightness);
    // calculate angle gamma between sun (zenith=0.0) and test point (theta,phi) :
    float cospsi = sin(theta) * sin(m_thetaSun) * cos(m_phiSun-phi) + cos(theta) * cos(m_thetaSun);
    float gamma = (cospsi > 1.0f) ? 0.0f : (cospsi < -1.0f) ? M_PI : acos(cospsi);
    // calculate vertex color:
    float A,B,C,D,E;
    vec3f skycolor_xyY;
	A =  0.17872f * m_turbidity - 1.46303f;
	B = -0.35540f * m_turbidity + 0.42749f;
	C = -0.02266f * m_turbidity + 5.32505f;
	D =  0.12064f * m_turbidity - 2.57705f;
	E = -0.06696f * m_turbidity + 0.37027f;
    skycolor_xyY[2] = expose(m_zenCol[2] * perez( A,B,C,D,E,theta,gamma ) / perez( A,B,C,D,E,0.0f,m_thetaSun ),0.04f); // scale luminance down using an exposure function
    
	A = -0.01925f * m_turbidity - 0.25922f;
	B = -0.06651f * m_turbidity + 0.00081f; 
	C = -0.00041f * m_turbidity + 0.21247f;
	D = -0.06409f * m_turbidity - 0.89887f;
	E = -0.00325f * m_turbidity + 0.04517f;
    skycolor_xyY[0] = m_zenCol[0] * perez( A,B,C,D,E,theta,gamma ) / perez( A,B,C,D,E,0.0f,m_thetaSun );
    
	A = -0.01669f * m_turbidity - 0.26078f;
	B = -0.09495f * m_turbidity + 0.00921f;
	C = -0.00792f * m_turbidity + 0.21023f;
	D = -0.04405f * m_turbidity - 1.65369f;
	E = -0.01092f * m_turbidity + 0.05291f;
    skycolor_xyY[1] = m_zenCol[1] * perez( A,B,C,D,E,theta,gamma ) / perez( A,B,C,D,E,0.0f,m_thetaSun );
    if(m_thetaSun<92.5f*DEG2RAD) return xyY2rgb(skycolor_xyY);
        
    float weight=((m_thetaSun*RAD2DEG)-92.5f)/5.0f;
    vec3f skycol(xyY2rgb(skycolor_xyY));
    skycol*= 1.0f-weight;
    return vec3f((weight*nightBrightness*f)+skycol[0],(weight*nightBrightness*f)+skycol[1],(weight*nightBrightness)+skycol[2]);
}

void SkyDome::draw(proCamera & camera) {
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glPushMatrix();
	glTranslatef(camera.pos()[X], camera.pos()[Y], camera.pos()[Z]-1.6f);

	for(unsigned int j=0; j<m_nVer-1; ++j) {
		glBegin(GL_QUAD_STRIP);
		for(unsigned int i=0; i<m_nHor; ++i) {
			glColor3fv(&m_vCol[i+m_nHor*j][X]);
			glVertex3fv(&m_vVtx[i+m_nHor*j][X]);
			glColor3fv(&m_vCol[i+m_nHor*(j+1)][X]);
			glVertex3fv(&m_vVtx[i+m_nHor*(j+1)][X]);
		}
		glColor3fv(&m_vCol[m_nHor*j][X]);
		glVertex3fv(&m_vVtx[m_nHor*j][X]);
		glColor3fv(&m_vCol[m_nHor*(j+1)][X]);
		glVertex3fv(&m_vVtx[m_nHor*(j+1)][X]);
		glEnd();
	}
	//draw sun:
	glEnable (GL_BLEND);
	glEnable (GL_TEXTURE_2D );
	glRotatef(m_phiSun*RAD2DEG+270.0f, 0,0,1);
	float sunPitch=90.0f-m_thetaSun*RAD2DEG;
	glRotatef(sunPitch, 1,0,0);
	float sunY=m_vVtx[0][X];
	float sunR=0.015f*sunY;
	glTranslatef(0, sunY, 0);
	vec4f sunCol(1.0f,1.0f,0.6f,1.0f);
	if(sunPitch<10.0f) {
	sunCol[1]*=0.75f+sunPitch*0.025f;
	sunCol[2]*=0.50f+sunPitch*0.05f;
	}
	glColor4fv(&sunCol[0]);
	glBindTexture(GL_TEXTURE_2D, m_sunId);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-sunR,0,-sunR);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(sunR,0,-sunR);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(sunR,0,sunR);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(-sunR,0,sunR);
	glEnd();
	glPopMatrix();
	glDisable (GL_TEXTURE_2D );
	glDisable (GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

vec4f SkyDome::sunVector() const {
	vec3f v;
	v.polar(sunHeading(), sunPitch());
	return vec4f(v[0], v[1], v[2], 0.0);
}

//--- class CloudLayer ---------------------------------------------

void CloudLayer::initGraphics() {
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    glEnableClientState(GL_VERTEX_ARRAY);
}

void CloudLayer::draw(proCamera & camera) {
	if(!m_visible) return;
	glPushMatrix();
	glTranslatef(camera.pos()[X], camera.pos()[Y], camera.pos()[Z]);
		
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glTranslatef(m_vel[X]*static_cast<float>(camera.time()), m_vel[Y]*static_cast<float>(camera.time()), 0.0f);
	glMatrixMode(GL_MODELVIEW);
	
	const float sz=20.0f;
	const float vtx[]={ 
		-2.0f*sz, -2.0f*sz, 0.0f,
		 2.0f*sz, -2.0f*sz, 0.0f, 
		-1.0f*sz, -1.0f*sz, 30.0f*sz/200.0f, 
		 1.0f*sz, -1.0f*sz, 30.0f*sz/200.0f, 
		-1.0f*sz,  1.0f*sz, 30.0f*sz/200.0f, 
		 1.0f*sz,  1.0f*sz, 30.0f*sz/200.0f, 
		-2.0f*sz,  2.0f*sz, 0.0f,
		 2.0f*sz,  2.0f*sz, 0.0f };
	const float tex[]={ 
		-2.0f, -2.0f,
		 2.0f, -2.0f, 
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f,  1.0f,
		-2.0f,  2.0f,
		 2.0f,  2.0f };
	float col[]={ 
		m_color[0], m_color[1], m_color[2], 0.0f,
		m_color[0], m_color[1], m_color[2], 0.0f,
		m_color[0], m_color[1], m_color[2], m_color[3],
		m_color[0], m_color[1], m_color[2], m_color[3],
		m_color[0], m_color[1], m_color[2], m_color[3],
		m_color[0], m_color[1], m_color[2], m_color[3],
		m_color[0], m_color[1], m_color[2], 0.0f,
		m_color[0], m_color[1], m_color[2], 0.0f };
	const unsigned int idx[]={
		2, 4, 5, 3,
		1, 0, 2, 3,
		0, 6, 4, 2,
		6, 7, 5, 4,
		7, 1, 3, 5 };

	glEnable (GL_TEXTURE_2D );
	glEnable (GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, m_texId);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY) ;
	glDisableClientState(GL_NORMAL_ARRAY) ;

	glTexCoordPointer ( 2, GL_FLOAT, 0, tex );
	glVertexPointer   ( 3, GL_FLOAT, 0, vtx );
	glColorPointer    ( 4, GL_FLOAT, 0, col );
	glDrawElements ( GL_QUADS, 20, GL_UNSIGNED_INT, idx );

	glEnableClientState(GL_NORMAL_ARRAY) ;
	glDisableClientState(GL_TEXTURE_COORD_ARRAY) ;
	glDisableClientState(GL_COLOR_ARRAY);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDisable (GL_BLEND);
	glDisable (GL_TEXTURE_2D );
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

}
