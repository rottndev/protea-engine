#ifndef _PRO_CANVAS_H
#define _PRO_CANVAS_H

#include <proResource.h>
class Canvas;
#include <set>
#include <string>
#include <vector>

//--- Element2D classes --------------------------------------------
/// abstract base class for elements attachable to a canvas
class Element2D {
public:
	/// constructor
	Element2D(const Canvas & root, const std::string & name = std::string() );
	/// destructor
	virtual ~Element2D() { ss_instance.erase(ss_instance.find(this)); }
	/// initializes element, e.g., calculates absolute coordinates
	virtual void init();
	/// draws element
	virtual void draw()=0;
	/// returns parent, const
	const Element2D* parent() const { return mp_parent; }
	/// returns parent
	Element2D* parent() { return mp_parent; }
	/// sets parent
	/** this method should not be called directly, but only from within ElementCanvas::append() */
	void parent(Element2D* pParent) { mp_parent = pParent; }
	/// returns name
	const std::string & name() const { return m_name; }
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
	/// returns root
	const Canvas & root() const { return m_root; }
	/// sets group, for search restriction
	void group(unsigned int id) { m_group=id; }
	/// returns group, for search restriction
	unsigned int group() const { return m_group; }
	
	/// returns primary color
	const Color & color() const { return m_color; }
	/// allows manipulating primary color
	Color & color() { return m_color; }
	/// sets primary color
	void color(float rIn, float gIn, float bIn, float aIn=1.0f) { m_color.value(rIn, gIn, bIn, aIn); }	
	/// sets primary color
	void color(const Color & c) { m_color=c; }	
	/// sets x position absolutely in pixels
	void x(float value) { m_pos[0]=value; m_rel &= ~POS_X; }
	/// sets y position absolutely in pixels
	void y(float value) { m_pos[1]=value; m_rel &= ~POS_Y; }
	/// sets x position relative to parent
	void xRel(float value) { m_pos[0]=value; m_rel |= POS_X; }
	/// sets y position relative to parent
	void yRel(float value) { m_pos[1]=value; m_rel |= POS_Y; }
	/// returns x position in pixels relative to parent
	float x() const { return (mp_parent&&(m_rel&POS_X)) ? m_pos[0]*mp_parent->width() : m_pos[0]; }
	/// returns y position in pixels relative to parent
	float y() const { return (mp_parent&&(m_rel&POS_Y)) ? m_pos[1]*mp_parent->height() : m_pos[1]; }
	/// returns absolute x position, if applicable
	float xAbs() const;
	/// returns absolute y position, if applicable
	float yAbs() const;
	/// sets width absolutely in pixels, if applicable
	virtual void width(float value) { m_dim[0]=value; m_rel &= ~DIM_X; }
	/// sets height absolutely in pixels, if applicable
	virtual void height(float value) { m_dim[1]=value; m_rel &= ~DIM_Y; }
	/// sets width relative to parent, if applicable
	virtual void widthRel(float value) { m_dim[0]=value; m_rel |= DIM_X; }
	/// sets height relative to parent, if applicable
	virtual void heightRel(float value) { m_dim[1]=value; m_rel |= DIM_Y; }
	/// returns width in pixels
	virtual float width() const { return (mp_parent&&(m_rel&DIM_X)) ? m_dim[0]*mp_parent->width() : m_dim[0]; }
	/// returns height in pixels
	virtual float height() const { return (mp_parent&&(m_rel&DIM_Y)) ? m_dim[1]*mp_parent->height() : m_dim[1]; }
	/// sets alignment
	void align(signed char hor, signed char ver) { m_align[0]=hor; m_align[1]=ver; }
	/// returns horizontal or vertical alignment
	signed char align(bool vertical=false) const { return m_align[vertical ? 1 : 0]; }
	/// symbolic names for alignments
	enum {
		ALIGN_CENTER = 0,
		ALIGN_LEFT = 1,
		ALIGN_BOTTOM = 1,
		ALIGN_RIGHT = -1,
		ALIGN_TOP = -1,
	};
	/// returns text/label, if available
	virtual std::string text() const { return std::string(); }
	/// sets text/label, if available
	virtual bool text(const std::string & s) { return false; }

	/// recursively searches for an Element2D having the passed name
	virtual Element2D* find(const std::string & name) { return name==m_name ? this : 0; }
	/// recursively searches for an Element2D at the passed absolute pixel position
	/** The implementation may not be pixel correct but rather be based on bounding boxes.
	  \param group (default=0, meaning all) allows to restrict the search to elements belonging to a previously defined group */
	virtual Element2D* find(float x, float y, unsigned int group=0);
	/// recursively searches for and deletes the subordinate Element2D having the passed name
	virtual bool erase(const std::string & name) { return false; }
	/// recursively searches for and deletes the passed Element2D
	virtual bool erase(Element2D* pElem) { return false; }
	/// recursively searches for and removes the passed Element2D without deleting the instance
	virtual Element2D* detach(Element2D* pElem) { return 0; }
	/// validates an Element2D pointer, returns 0 in case of an invalid pointer, otherwise the passed pointer
	static Element2D* validate(Element2D* pElem);
protected:
	/// root reference
	const Canvas & m_root;
	/// pointer to parent Element2D
	Element2D* mp_parent;
	/// stores group, for search and filtering
	unsigned int m_group;
	/// stores position as passed by the user
	float m_pos[2];
	/// stores dimensions as passed by the user
	float m_dim[2];
	/// stores absolute position & dimensions considering alignment and parent dimensions
	float m_bounding[4]; 
	/// stores horizontal and vertical alignment
	signed char m_align[2];
	/// stores flags defining whether attributes are absolute or relative to parent dimensions
	unsigned int m_rel;
	/// symbolic names of attributes that can be absolute or relative
	enum {
		POS_X = 1,
		POS_Y = (1<<1),
		DIM_X = (1<<2),
		DIM_Y = (1<<3),
	};
	/// stores (primary) color
	Color m_color;
	/// user-definable name
	std::string m_name;
	/// stores set of valid pointers, used by validate()
	static std::set<Element2D*> ss_instance;
};

/// filled rectangle 2D element
class ElementRect: public Element2D {
public:
	/// constructor
	ElementRect(const Canvas & root, const std::string & name = std::string() ) : Element2D(root, name) { }
	/// draws element
	virtual void draw();
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
};

/// filled rounded rectangle 2D element
class ElementPanel: public Element2D {
public:
	/// constructor
	ElementPanel(const Canvas & root, const std::string & name = std::string() ) : 
		Element2D(root, name), m_radius(8.0f), m_corner(CORNER_NONE), m_borderWidth(0.0f), m_borderColor(Color::transparent) { }
	/// draws element
	virtual void draw();
	/// initializes element
	virtual void init();
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;

	/// symbolic names of individual corners
	enum {
		CORNER_NONE = 0,
		CORNER_LEFT_BOTTOM = 1,
		CORNER_RIGHT_BOTTOM = 2,
		CORNER_RIGHT_TOP = 4,
		CORNER_LEFT_TOP = 8,
		CORNER_BOTTOM = 3,
		CORNER_RIGHT = 6,
		CORNER_TOP = 12,
		CORNER_LEFT = 9,
		CORNER_ALL = 15,
	};
	/// sets corner radius
	void cornerRadius(float f) { m_radius = f; }
	/// returns corner radius
	float cornerRadius() const { return m_radius; }
	/// allows individual rounding of corners
	void corners(unsigned int c=CORNER_ALL) { m_corner = c; }
	/// returns rounded corners bit-encoded
	unsigned int corners() const { return m_corner; }

	/// sets border width
	void borderWidth(float f) { m_borderWidth = f; }
	/// returns border width
	float borderWidth() const { return m_borderWidth; }
	/// returns border color
	const Color & borderColor() const { return m_borderColor; }
	/// allows manipulating border color
	Color & borderColor() { return m_borderColor; }
	/// sets border color
	void borderColor(const Color & c) { m_borderColor=c; }	

protected:
	/// stores corner radius
	float m_radius;
	/// stores corners to be rounded
	unsigned int m_corner;
	/// stores border width
	float m_borderWidth;
	/// stores border color
	Color m_borderColor;
	/// vertex buffer
	std::vector<float> mv_vtx;
};

/// straight line segment 2D element
class ElementLine: public Element2D {
public:
	/// constructor
	ElementLine(const Canvas & root, const std::string & name = std::string() ) : Element2D(root, name), m_wLine(1.0f) { 
		ma_vtx[0]=ma_vtx[1]=ma_vtx[2]=ma_vtx[3]=1.0f; }
	/// draws element
	virtual void draw();
	/// initializes element, e.g., calculates absolute coordinates
	virtual void init();
	/// sets line width
	void lineWidth(float w) { m_wLine = w; }
	/// returns line width
	float lineWidth() const { return m_wLine; }
	
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
protected:
	/// stores line width
	float m_wLine;
	/// stores vertices
	float ma_vtx[4];
};

/// a simple one-line text string 2D element
class ElementString: public Element2D {
public:
	/// constructor
	ElementString(const Canvas & root, const Font & font, const std::string & name = std::string() ) : Element2D(root, name), m_font(font) { }
	/// draws element
	virtual void draw();
	/// returns text
	virtual std::string text() const { return m_text; }
	/// sets text
	virtual bool text(const std::string & s);
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
	/// returns font
	const Font & font() const { return m_font; }
protected:
	/// stores font
	const Font & m_font;
	/// stores text
	std::string m_text;
};

/// a multi-line text 2D element allowing for text alignment
class ElementText: public ElementString {
public:
	/// constructor
	ElementText(const Canvas & root, const Font & font, const std::string & name = std::string() ) : 
		ElementString(root, font, name), m_alignText(1.0f), m_lineHeight(1.0f) { widthRel(1.0f); heightRel(1.0f); }
	/// draws element
	virtual void draw();
	/// returns text
	virtual std::string text() const { return m_text; }
	/// sets text
	virtual bool text(const std::string & s) { m_text = s; init(); return true; }
	/// sets horizontal text alignment
	void alignText(signed char hor) { m_alignText=hor>0 ? 1.0f : hor<0 ? -1.0f : 0.0f; }
	/// returns horizontal text alignment
	signed char alignText() const { return m_alignText>0.0f ? ALIGN_LEFT : m_alignText<0.0f ? ALIGN_RIGHT : ALIGN_CENTER; }
	/// sets line height factor
	void lineHeight(float h) { m_lineHeight = h; }
	/// returns line height factor
	float lineHeight() const { return m_lineHeight; }

	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
protected:
	/// stores text alignment
	float m_alignText;
	/// stores line height factor
	float m_lineHeight;
};

/// pixel image 2D element
class ElementImg: public Element2D {
public:
	/// constructor
	ElementImg(const Canvas & root, unsigned int texId, unsigned int texWidth, unsigned int texHeight, unsigned int texDepth, const std::string & name = std::string() );
	/// draws element
	virtual void draw();
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
	/// returns texture width in pixels
	unsigned int textureWidth() const { return m_texWidth; }
	/// returns texture height in pixels
	unsigned int textureHeight() const { return m_texHeight; }
	/// returns texture depth in bytes
	unsigned int textureDepth() const { return m_texDepth; }
protected:
	/// stores texture ID
	unsigned int m_texId;
	/// stores texture width
	unsigned int m_texWidth;
	/// stores texture height
	unsigned int m_texHeight;
	/// stores texture pixel depth in bytes
	unsigned int m_texDepth;
};

//--- class ElementCanvas ------------------------------------------

/// container element for Element2D instances which can be placed absolutely or relatively
class ElementCanvas: public ElementRect {
public:
	/// constructor
	ElementCanvas(const Canvas & root, const std::string & name = std::string() );
	/// destructor
	virtual ~ElementCanvas() { clear(); }
	/// initializes element, e.g., calculates absolute coordinates
	virtual void init();
	/// draws element
	virtual void draw();
	/// returns number of direct subordinate Elements
	size_t size() const { return mv_elem.size(); }
	/// appends an element
	/** the lifetime of the appended Element2D is managed by the parent afterwards. Passed null pointers are ignored. */
	virtual Element2D * append(Element2D * pElem) { 
		if(pElem) { mv_elem.push_back(pElem); pElem->parent(this); } return pElem; }
	/// element access by index
	Element2D* operator[](unsigned int n) const { return n>=mv_elem.size() ? 0 : mv_elem[n]; }
	/// element access by name
	Element2D* operator[](const std::string & name) const;
	/// recursively searches for an Element2D having the passed name
	virtual Element2D* find(const std::string & name);
	/// recursively searches for an Element2D at the passed absolute pixel position
	/** The implementation may not be pixel correct but rather be based on bounding boxes.
	  \param group (default=0, meaning all) allows to restrict the search to elements belonging to a previously defined group */
	virtual Element2D* find(float x, float y, unsigned int group=0);
	/// recursively searches for and deletes the Element2D having the passed name
	virtual bool erase(const std::string & name);
	/// recursively searches for and deletes the passed Element2D
	virtual bool erase(Element2D* pElem);
	/// recursively searches for and removes the passed Element2D without deleting the instance
	virtual Element2D* detach(Element2D* pElem);
	/// clears all Element2D children
	virtual void clear();
	
	/// sets global transformation by a rotation (in degrees)
	void rotate(float angle) { m_rot = angle; }
	/// returns rotation (in degrees)
	float rotate() const { return m_rot; }
	/// sets global transformation by a scale factor
	void scale(float scaleX, float scaleY) { if(scaleX) m_scale[0] = scaleX; if(scaleY) m_scale[1] = scaleY; }
	/// returns scale factor
	float scale(unsigned int dimension=0) { return m_scale[dimension]; }
		
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
protected:
	/// converts a passed global coordinate to a local coordinate
	void toLocal(float & xIn, float & yIn) const;

	/// stores Element2D children
	std::vector<Element2D*> mv_elem;
	/// stores rotation angle
	float m_rot;
	/// stores scaling factors
	float m_scale[2];
};

//--- class Canvas -------------------------------------------------

#include "proDevice.h"

/// root canvas for 2D element  based visualizations
class Canvas : public ElementCanvas, public ListenerResize {
public:
	/// constructor
	Canvas(DeviceWindow & window, const std::string & name = std::string());
	/// destructor
	virtual ~Canvas() { m_wnd.unregListenerResize(this); }
	/// draws canvas
	virtual void draw();
	/// window resize event listener
	virtual void resize( float width, float height) { m_dim[0]=width; m_dim[1]=height; ElementCanvas::init(); }
	/// returns width in pixels
	virtual float width() const { return (m_rel&DIM_X) ? m_dim[0]*m_wnd.width() : m_dim[0]; }
	/// returns height in pixels
	virtual float height() const { return (m_rel&DIM_Y) ? m_dim[1]*m_wnd.height() : m_dim[1]; }
		
	/// returns font resource associated with this name
	const Font* font(const char* name=0) const { return name ? m_wnd.font(name) : m_wnd.font(); }
	/// returns font resource associated with this name
	const Font* font(const std::string & name) const { return name.size() ? m_wnd.font(name.c_str()) : m_wnd.font(); }
	/// returns properties of a previously loaded texture by name
	bool textureProperties(const std::string & name, unsigned int & id, unsigned int & width, unsigned int & height,  unsigned int & depth) const {
		return m_wnd.textureProperties(name, id, width, height, depth); }
	/// returns the color associated with this name, or black in case no color is associated
	const Color & color(const std::string & name) const { return m_wnd.color(name); }

	/// sets the current cursor visualization
	void cursor(Element2D* pCursor);
	/// returns the current cursor visualization
	Element2D* cursor() { return mp_cursor; }
	/// returns the current cursor visualization, const
	const Element2D* cursor() const { return mp_cursor; }		
protected:
	/// host window reference
	DeviceWindow & m_wnd;
	/// stores (mouse) cursor visualization
	Element2D* mp_cursor;
};

#endif //_PRO_CANVAS_H
