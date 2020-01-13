#include "proCanvas.h"
#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif
#include <GL/gl.h>
#include <cmath>

using namespace std;

//--- class Element2D ----------------------------------------------

const char* Element2D::s_type = "element";
set<Element2D*> Element2D::ss_instance;

Element2D::Element2D(const Canvas & root, const std::string & name) : m_root(root), mp_parent(0), m_group(0), m_rel(0), m_name(name) { 
	m_pos[0] = m_pos[1] = 0.0f; 
	m_dim[0]=m_dim[1]=0.0f; 
	m_align[0]=ALIGN_LEFT; 
	m_align[1]=ALIGN_BOTTOM;
	m_bounding[0] = m_bounding[1] = m_bounding[2] = m_bounding[3] = 0.0f;
	ss_instance.insert(this); 
}

void Element2D::init() { 
	//printf("%s %s x:%.2f y:%.2f w:%.2f h:%.2f alignX:%i alignY:%i\n", type(), name().c_str(), m_pos[0], m_pos[1], m_dim[0], m_dim[1], m_align[0], m_align[1]);
	m_bounding[0] = ((mp_parent&&(m_rel&POS_X)) ? m_pos[0]*mp_parent->width() : m_pos[0]) + (0.5f*width()*((float)m_align[0]-1.0f));
	m_bounding[1] = ((mp_parent&&(m_rel&POS_Y)) ? m_pos[1]*mp_parent->height() : m_pos[1]) + (0.5f*height()*((float)m_align[1]-1.0f));
	m_bounding[2] = m_bounding[0] + ((mp_parent&&(m_rel&DIM_X)) ? m_dim[0]*mp_parent->width() : m_dim[0]);
	m_bounding[3] = m_bounding[1] + ((mp_parent&&(m_rel&DIM_Y)) ? m_dim[1]*mp_parent->height() : m_dim[1]);
}

Element2D* Element2D::validate(Element2D* pElem) {
	set<Element2D*>::const_iterator it = ss_instance.find(pElem);
	return it==ss_instance.end() ? 0 : pElem;
}

Element2D* Element2D::find(float x, float y, unsigned int group) {
	if(group&&(group!=m_group)) return 0;
	return ((x>=m_bounding[0])&&(x<m_bounding[2])&&(y>=m_bounding[1])&&(y<m_bounding[3])) ? this : 0;
}

float Element2D::xAbs() const {
	float x = this->x();
	Element2D* pElem = mp_parent;
	while(pElem&&(pElem!=&m_root)) {
		x+=pElem->x();
		pElem = pElem->parent();
	}
	return x;
}

float Element2D::yAbs() const {
	float y = this->y();
	Element2D* pElem = mp_parent;
	while(pElem&&(pElem!=&m_root)) {
		y+=pElem->y();
		pElem = pElem->parent();
	}
	return y;
}

//--- class ElementRect --------------------------------------------

const char* ElementRect::s_type = "rectangle";

void ElementRect::draw() {
	glBegin(GL_QUADS);
	glColor4fv(m_color.value());
	glVertex2f(m_bounding[0], m_bounding[1]);
	glVertex2f(m_bounding[2], m_bounding[1]);
	glVertex2f(m_bounding[2], m_bounding[3]);
	glVertex2f(m_bounding[0], m_bounding[3]);
	glEnd();	
}

//--- class ElementPanel -------------------------------------------

const char* ElementPanel::s_type = "panel";

void ElementPanel::draw() {
	glBegin(GL_TRIANGLE_FAN);
	glColor4fv(m_color.value());
	for(size_t i=0; i<mv_vtx.size(); i+=2) glVertex2fv(&mv_vtx[i]);
	glEnd();
	if(m_borderWidth && (m_borderColor.a>0.0f)) {
		glLineWidth(m_borderWidth);
		glBegin(GL_LINE_LOOP);
		glColor4fv(m_borderColor.value());
		for(size_t i=0; i<mv_vtx.size(); i+=2) glVertex2fv(&mv_vtx[i]);
		glEnd();
		glLineWidth(1.0f);
	}
}

void ElementPanel::init() {
	Element2D::init();
	mv_vtx.clear();
	const unsigned int nPoly=32;
	const float dAngle = 2.0f*M_PI/nPoly;
	
	if(m_corner&CORNER_LEFT_BOTTOM) {
		float c[] ={m_bounding[0]+m_radius, m_bounding[1]+m_radius};
		mv_vtx.push_back(m_bounding[0]);
		mv_vtx.push_back(m_bounding[1]+m_radius);
		for(float angle=M_PI+dAngle; angle<1.5f*M_PI; angle+=dAngle) {
			mv_vtx.push_back(c[0] + static_cast<float>(cos(angle))*m_radius);
			mv_vtx.push_back(c[1] + static_cast<float>(sin(angle))*m_radius);
		}
		mv_vtx.push_back(m_bounding[0]+m_radius);
		mv_vtx.push_back(m_bounding[1]);
	}
	else {
		mv_vtx.push_back(m_bounding[0]);
		mv_vtx.push_back(m_bounding[1]);
	}
		
	if(m_corner&CORNER_RIGHT_BOTTOM) {
		float c[]= { m_bounding[2]-m_radius, m_bounding[1]+m_radius };
		mv_vtx.push_back(m_bounding[2]-m_radius);
		mv_vtx.push_back(m_bounding[1]);
		for(float angle=1.5*M_PI+dAngle; angle<2*M_PI; angle+=dAngle) {
			mv_vtx.push_back(c[0]+static_cast<float>(cos(angle))*m_radius);
			mv_vtx.push_back(c[1]+static_cast<float>(sin(angle))*m_radius);
		}
		mv_vtx.push_back(m_bounding[2]);
		mv_vtx.push_back(m_bounding[1]+m_radius);
	}
	else {
		mv_vtx.push_back(m_bounding[2]);
		mv_vtx.push_back(m_bounding[1]);
	}
		
	if(m_corner&CORNER_RIGHT_TOP) {
		float c[]={m_bounding[2]-m_radius, m_bounding[3]-m_radius };
		mv_vtx.push_back(m_bounding[2]);
		mv_vtx.push_back(m_bounding[3]-m_radius);
		for(float angle=dAngle; angle<0.5*M_PI; angle+=dAngle) {
			mv_vtx.push_back(c[0]+static_cast<float>(cos(angle))*m_radius);
			mv_vtx.push_back(c[1]+static_cast<float>(sin(angle))*m_radius);
		}
		mv_vtx.push_back(m_bounding[2]-m_radius);
		mv_vtx.push_back(m_bounding[3]);
	}
	else {
		mv_vtx.push_back(m_bounding[2]);
		mv_vtx.push_back(m_bounding[3]);
	}
		
	if(m_corner&CORNER_LEFT_TOP) {
		float c[]={m_bounding[0]+m_radius, m_bounding[3]-m_radius};
		mv_vtx.push_back(m_bounding[0]+m_radius);
		mv_vtx.push_back(m_bounding[3]);
		for(float angle=0.5*M_PI+dAngle; angle<M_PI; angle+=dAngle) {
			mv_vtx.push_back(c[0]+static_cast<float>(cos(angle))*m_radius);
			mv_vtx.push_back(c[1]+static_cast<float>(sin(angle))*m_radius);
		}
		mv_vtx.push_back(m_bounding[0]);
		mv_vtx.push_back(m_bounding[3]-m_radius);
	}
	else {
		mv_vtx.push_back(m_bounding[0]);
		mv_vtx.push_back(m_bounding[3]);
	}
	//cout << m_bounding[0] << " " << m_bounding[1] << " " << m_bounding[2] << " " << m_bounding[3] << endl;
}

//--- class ElementLine --------------------------------------------

const char* ElementLine::s_type = "line";

void ElementLine::init() { 
	//printf("%s %s x:%.2f y:%.2f w:%.2f h:%.2f alignX:%i alignY:%i\n", type(), name().c_str(), m_pos[0], m_pos[1], m_dim[0], m_dim[1], m_align[0], m_align[1]);
	ma_vtx[0] = ((mp_parent&&(m_rel&POS_X)) ? m_pos[0]*mp_parent->width() : m_pos[0]) + (0.5f*width()*((float)m_align[0]-1.0f));
	ma_vtx[1] = ((mp_parent&&(m_rel&POS_Y)) ? m_pos[1]*mp_parent->height() : m_pos[1]) + (0.5f*height()*((float)m_align[1]-1.0f));
	ma_vtx[2] = ma_vtx[0] + ((mp_parent&&(m_rel&DIM_X)) ? m_dim[0]*mp_parent->width() : m_dim[0]);
	ma_vtx[3] = ma_vtx[1] + ((mp_parent&&(m_rel&DIM_Y)) ? m_dim[1]*mp_parent->height() : m_dim[1]);
	if(ma_vtx[0]<ma_vtx[2]) {
		m_bounding[0] = ma_vtx[0];
		m_bounding[2] = ma_vtx[2];
	}
	else {
		m_bounding[0] = ma_vtx[2];
		m_bounding[2] = ma_vtx[0];
	}
	if(ma_vtx[1]<ma_vtx[3]) {
		m_bounding[1] = ma_vtx[1];
		m_bounding[3] = ma_vtx[3];
	}
	else {
		m_bounding[1] = ma_vtx[3];
		m_bounding[3] = ma_vtx[1];
	}
}

void ElementLine::draw() {
	glLineWidth(m_wLine);
	glBegin(GL_LINES);
	glColor4fv(m_color.value());
	glVertex2fv(&ma_vtx[0]);
	glVertex2fv(&ma_vtx[2]);
	glEnd();	
	glLineWidth(1.0f);
}

//--- class ElementString ------------------------------------------

const char* ElementString::s_type = "string";

void ElementString::draw() {
	if(!m_text.size()) return;
	glColor4fv(m_color.value());
	glEnable (GL_TEXTURE_2D );
	m_font.print( m_bounding[0], m_bounding[1], m_text.c_str(), m_text.size());
	glDisable (GL_TEXTURE_2D );	
}

bool ElementString::text(const std::string & s) { 
	m_text = s; 
	width(m_font.width(s.c_str()));
	height(m_font.height(s.c_str()));
	init();
	return true;
}

//--- class ElementText --------------------------------------------

const char* ElementText::s_type = "text";

void ElementText::draw() {
	if(!m_text.size()) return;
	glColor4fv(m_color.value());
	glEnable (GL_TEXTURE_2D );
	float posX = (m_alignText == ALIGN_LEFT) ? m_bounding[0] : (m_alignText == ALIGN_RIGHT) ? m_bounding[2] : 0.5f*(m_bounding[0]+m_bounding[2]);
	m_font.print( posX, m_bounding[3], m_bounding[2]-m_bounding[0], m_bounding[3]-m_bounding[1], 
		m_text.c_str(), m_alignText, m_lineHeight);
	glDisable (GL_TEXTURE_2D );	
}

//--- class ElementImg ---------------------------------------------

const char* ElementImg::s_type = "image";

ElementImg::ElementImg(const Canvas & root, unsigned int texId, unsigned int texWidth, unsigned int texHeight, unsigned int texDepth, const std::string & name) : 
	Element2D(root, name), m_texId(texId), m_texWidth(texWidth), m_texHeight(texHeight), m_texDepth(texDepth) { 
	m_dim[0] = static_cast<float>(m_texWidth);
	m_dim[1] = static_cast<float>(m_texHeight);
}

void ElementImg::draw() {
	if(!m_texId) return;
	glEnable (GL_TEXTURE_2D );
	glBindTexture(GL_TEXTURE_2D, m_texId);
	glBegin(GL_QUADS);
	glColor4fv(m_color.value());
	glTexCoord2f(0.0f,0.0f);	
	glVertex2f(m_bounding[0], m_bounding[1]);
	glTexCoord2f(1.0f,0.0f);
	glVertex2f(m_bounding[2], m_bounding[1]);
	glTexCoord2f(1.0f,1.0f);
	glVertex2f(m_bounding[2], m_bounding[3]);
	glTexCoord2f(0.0f,1.0f);
	glVertex2f(m_bounding[0], m_bounding[3]);
	glEnd();
	glDisable (GL_TEXTURE_2D );	
}

//--- class ElementCanvas ------------------------------------------

const char* ElementCanvas::s_type = "canvas";

ElementCanvas::ElementCanvas(const Canvas & root, const std::string & name) : ElementRect(root, name), m_rot(0.0f) { 
	m_color.a=0.0f;
	m_scale[0] = m_scale[1] = 1.0f;
}

void ElementCanvas::init() {
	ElementRect::init();
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it)
		(*it)->init();
}

void ElementCanvas::draw() {
	glPushMatrix();
	float posX = x(), posY= y();
	glTranslatef(posX, posY, 0.0f);
	glRotatef(m_rot, 0.0f, 0.0f,1.0f);
	glScalef(m_scale[0],m_scale[1],1.0f);
	
	glTranslatef(-posX, -posY, 0.0f);
	ElementRect::draw();
	
	glTranslatef(m_bounding[0], m_bounding[1], 0.0f);
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it)
		(*it)->draw();
	glPopMatrix();
}

Element2D* ElementCanvas::operator[](const std::string & name) const {
	for(vector<Element2D*>::const_iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it)
		if((*it)->name()==name) return *it;
	return 0;
}

Element2D* ElementCanvas::find(const std::string & name) {
	if(name==m_name) return this;
	for(vector<Element2D*>::const_iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
		Element2D* pElem = (*it)->find(name);
		if(pElem) return pElem;
	}
	return 0;
}

void ElementCanvas::toLocal(float & xIn, float & yIn) const{
	xIn/=m_scale[0]; 
	yIn/=m_scale[1];
	float a = -m_rot*M_PI/180.0f;
	float xTmp = xIn*cos(a)-yIn*sin(a);
	yIn = xIn*sin(a) + yIn*cos(a) - m_bounding[1];
	xIn = xTmp - m_bounding[0]; 
}


Element2D* ElementCanvas::find(float xIn, float yIn, unsigned int group) {
	// do reverse transformation:
	float xLocal=xIn; 
	float yLocal=yIn;
	toLocal(xLocal, yLocal);
	//  traverse in reverse order simulating display order:
	for(vector<Element2D*>::reverse_iterator it=mv_elem.rbegin(); it<mv_elem.rend(); ++it) {
		Element2D* pElem = (*it)->find(xLocal,yLocal,group);
		if(pElem) return pElem;
	}
	return Element2D::find(xIn,yIn,group);
}

bool ElementCanvas::erase(const std::string & name) {
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
		if((*it)->name()==name) {
			delete *it;
			mv_elem.erase(it);
			return true;
		}
		if((*it)->erase(name)) return true;
	}
	return false;
}

bool ElementCanvas::erase(Element2D* pElem) {
	if(!detach(pElem)) return false;
	delete pElem;
	return true;
}

Element2D* ElementCanvas::detach(Element2D* pElem) {
	if(!pElem) return 0;
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
		if((*it)==pElem) {
			Element2D* ret = *it;
			mv_elem.erase(it);
			ret->parent(0);
			return ret;
		}
		Element2D* ret = (*it)->detach(pElem);
		if(ret) return ret;
	}
	return 0;
}

void ElementCanvas::clear() {
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it)
		delete *it;
	mv_elem.clear();
}

//--- class Canvas -----------------------------------------

Canvas::Canvas(DeviceWindow & window, const std::string & name) : ElementCanvas(*this, name), m_wnd(window), mp_cursor(0) { 
	m_wnd.regListenerResize(this);
	m_dim[0] = static_cast<float>(window.width());
	m_dim[1] = static_cast<float>(window.height());
}

void Canvas::draw() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,m_wnd.width(),0,m_wnd.height(),-1.0f,1.0f);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	
	ElementCanvas::draw();
	if(mp_cursor) mp_cursor->draw();

	glDisable (GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}

void Canvas::cursor(Element2D* pCursor) { 
	if(mp_cursor) mp_cursor->parent(0); 
	mp_cursor=pCursor; 
	if(!mp_cursor) return;
	mp_cursor->parent(this); 
	mp_cursor->align(ALIGN_LEFT, ALIGN_TOP);
}

