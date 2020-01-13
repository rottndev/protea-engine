#ifndef _PRO_GUI_H
#define _PRO_GUI_H

#include "proCanvas.h"
#include <proDevice.h>
#include <proCallable.h>

#include <vector>
#include <string>
#include <map>

//--- class Widget -------------------------------------------------

/// base class for GUI widgets
class Widget : public ElementCanvas {
public:
	/// symbolic names for widget activity states
	enum {
		STATE_DISABLED = 0,
		STATE_NORMAL,
		STATE_FOCUS,
		STATE_TRIGGERED,
		STATE_COUNTER /// not a regular state, but a useful constant providing the total number of states
	};
	/// symbolic names for widget events
	enum {
		EVT_ENTER = 0,
		EVT_NEXT,
		EVT_PREV,
		EVT_CANCEL,
		EVT_COUNTER /// not a regular event, but a useful constant providing the total number of events
	};
	/// constructor
	Widget(const Canvas & root, const std::string & name = std::string() );
	/// destructor
	virtual ~Widget() { if(mp_background) delete mp_background; }
	/// initializes element, e.g., calculates absolute coordinates
	virtual void init();
	/// draws element
	virtual void draw();
	/// returns pointer to background, const
	const Element2D* background() const { return mp_background; }
	/// returns pointer to background
	Element2D* background() { return mp_background; }
	/// sets background
	void background(Element2D* pBackground) { mp_background=pBackground; if(mp_background) mp_background->parent(this); }
	/// sets width absolutely in pixels, if applicable
	virtual void width(float value) { ElementCanvas::width(value); if(mp_background) mp_background->widthRel(1.0f); }
	/// sets height absolutely in pixels, if applicable
	virtual void height(float value) { ElementCanvas::height(value); if(mp_background) mp_background->heightRel(1.0f); }
	/// returns width in pixels
	virtual float width() const { return ElementCanvas::width(); }
	/// returns height in pixels
	virtual float height() const { return ElementCanvas::height(); }

	/// updates widget based on pointer input
	virtual int update(float pointerX, float pointerY, int buttonDown);
	/// updates widget based on time
	virtual void update(double deltaT);
	/// returns command or code value associated with this widget
	virtual std::string value() const { return m_value; }
	/// sets command or code value associated with this widget
	void value(const std::string & s) { m_value=s; }
	///returns the code value or command associated with an event of this widget
	const std::string & event(unsigned int evt) const { return ma_evt[evt]; }
	/// sets a code value or command associated with an event of this widget
	void event(unsigned int evt, const std::string & s) { ma_evt[evt]=s; }
	/// returns state
	int state() const { return m_state; }
	/// sets state
	virtual void state(int st) { m_state = st; }
	/// returns style name
	const std::string & style() const { return m_style; }
	/// sets style name
	virtual void style(const std::string & s);
	/// returns true if the Widget is a modal widget
	/** modal Widgets automatically take the focus, any further input propagation is suppressed until the Widget is deleted */
	virtual bool modal() const { return false; }

	/// switches input focus to next subelement if applicable
	virtual Widget* next() { return 0; }
	/// switches input focus to previous subelement if applicable
	virtual Widget* previous() { return 0; }
	/// switches input focus one level down if applicable
	virtual Widget* enter() { return 0; }
	/// cleans up before switching input focus up, if applicable
	virtual void leave() { state(Widget::STATE_NORMAL); }

protected:
	/// stores state
	int m_state;
	/// stores widget value/command
	std::string m_value;
	/// stores event values/commands
	std::string ma_evt[EVT_COUNTER];
	/// stores style name
	std::string m_style;
	/// pointer to background visualization element
	Element2D* mp_background;
};

//--- class WidgetStyle --------------------------------------------

/// a class holding style properties
class WidgetStyle{
public:
	/// default constructor
	WidgetStyle();
	/// symbolic names for typical color usage types
	enum {
		COLOR_DEFAULT = 0,
		COLOR_FOREGR = 0,
		COLOR_BACKGR = 1,
		COLOR_BORDER = 2,
		COLOR_COUNTER = 3,
	};
	/// returns a color
    const Color & color(int type, int activityState = Widget::STATE_NORMAL) const;
	/// sets a color
    void color(int type, int activityState, const Color & color)  {
		if(type<COLOR_COUNTER) mv_color[type]=color; }
	/// returns font
    const Font & font() const;
	/// sets font
    void font(const Font & f) { mp_font=&f; }
	/// returns a measure
    float measure(int type) const; 
	/// sets a measure
    void measure(int type, float value) {
		if(type<MEASURE_COUNTER) mv_measure[type]=value; }
	/// symbolic names for measure types
	enum {
		PADDING = 0, 
		SPACING,
		CORNER_RADIUS,
		BORDER_WIDTH,
		MEASURE_COUNTER,
	};
	/// interprets a Var-based style definition
	bool interpret(const Var & v);	
	/// sets a parent style providing fallbacks for all undefined style characteristics
	void parent(const WidgetStyle * style) { mp_parent=style; }
	/// returns parent style
	const WidgetStyle * parent() const { return mp_parent; }
	/// sets a global default style providing fallbacks for all undefined style characteristics
	static void defaultStyle(WidgetStyle & style);
	/// returns global default style
	static const WidgetStyle & defaultStyle();
protected:
	/// stores colors
	Color mv_color[COLOR_COUNTER*Widget::STATE_COUNTER];
	/// stores font
	const Font* mp_font;
	/// stores measures
	float mv_measure[MEASURE_COUNTER];
	/// pointer to default style
	static WidgetStyle* sp_default;
	/// pointer to parent style
	const WidgetStyle* mp_parent;
};

//--- class CanvasGui ----------------------------------------------

class WidgetInput;

/// GUI root element
class CanvasGui : public Canvas, public Callable {
public:
	/// constructor
	CanvasGui(DeviceWindow & window, const DeviceInput & devInput, const Var & ini = Var::null);
	/// destructor
	virtual ~CanvasGui() { clear(); }
	/// generic method calling the object instance to evalute the provided command
	virtual Var call(const std::string & cmd, const Var & arg);
	/// returns all keys/command names provided by this Callable as Var::ARRAY
	virtual Var info() const {
		return Var().append("append").append("erase").append("clear").append("pageNext").append("pagePrev")
		.append("focus").append("value").append("text").append("load").append("inputByPointer"); }
		
	/// updates input state
	int update(double deltaT=0.0);
	/// updates visualization
	virtual void draw();
	/// returns command or code value of currently selected widget or an empty string in case no event has been triggered
	const std::string & value() const { return m_value; }
	/// returns state
	int state() const { return m_state; }
	/// returns true if input should be exclusively dispatched to GUI
	bool grabInput() const { return mp_modal ? true : (m_inputDpad&&(m_state==Widget::STATE_FOCUS)) ? true : false; }
	/// returns pointer to currently focused Widget
	const Widget* focus() const;
	/// sets focus by Widget name
	const Widget* focus(const std::string & name);
	/// sets current text input widget
	void textInput(WidgetInput* pInput);

	/// allows defining the source for (mouse) pointer GUI control, default is DeviceWindow, axes 3&4 for input position, and button 0 for triggering events
	void inputPointerSource(const DeviceInput & source, unsigned int axisX=0, unsigned int axisY=1, unsigned int buttonId = 0);
	/// allows defining the source for D-pad GUI control
	void inputDpadSource(const DeviceInput & source, unsigned int buttonIdNext, unsigned int buttonIdPrev, unsigned int buttonIdOk, unsigned int buttonIdCancel);
	/// returns true if the GUI currently responses to (mouse) pointer input
	bool inputByPointer() const { return m_inputDpad==false; }
	/// turns (mouse) pointer input mode on or off, switches between DPad mode and mouse mode
	void inputByPointer(bool yesno) { m_inputDpad=!yesno; }

	/// registers a C++ function based Element2D factory 
	void registerElementFactory(Element2D* (*factoryFunc)(const Var&, const Canvas &), const std::string & typeName) {
		mm_elementFactoryCFunc.insert(make_pair(typeName, factoryFunc)); }
	/// registers a Var based Element2D factory 
	void registerElementFactory(const Var & v, const std::string & typeName) {
		mm_elementFactoryVar.insert(make_pair(typeName, v)); }
	/// creates an Element2D instance based on a Var
	Element2D* createElement(const Var & v) const;
		
	/// appends a single element or widget based on a Var::MAP object or multiple widgets defined in a Var::ARRAY
	Element2D* append(const Var & v);
	/// appends an element or widget
	/** the lifetime of the appended Element2D is managed by the parent afterwards. Passed null pointers are ignored. */
	virtual Element2D * append(Element2D * pElem);
	/// recursively searches for and deletes the Element2D having the passed name
	virtual bool erase(const std::string & name) { return erase(find(name)); }
	/// recursively searches for and deletes the passed Element2D
	virtual bool erase(Element2D* pElem);
	/// clears all Element2D and Widget children
	virtual void clear();
	/// recursively searches for and removes the passed Element2D without deleting the instance
	virtual Element2D* detach(Element2D* pElem);
	
	/// returns style by name
	const WidgetStyle & style(const std::string & name="default") const;
	/// returns true if a style exists
	bool styleExists(const std::string & name) const {
		return (mm_style.find(name)==mm_style.end()) ? false : true; }
protected:
	/// initializes resources
	void initResources();
	/// registers Element2D factories based on Var templates
	void registerElementFactories(const Var & factoryTable);
	/// updates GUI based on DPad input
	int updateByDpad();
	/// updates GUI based on pointer input
	int updateByPointer();

	/// stores state
	int m_state;
	/// stores current value/command
	std::string m_value;
	/// stores pointer to currently focused Widget
	Widget* mp_focus;
	/// stores pointer to current modal Widget
	Widget* mp_modal;
	/// stores pointer to current text input widget
	WidgetInput* mp_textInput;
	/// iterator pointing to currently focused child element
	std::vector<Element2D*>::iterator mi_elem;

	/// stores whether current input comes from pointer or DPad
	bool m_inputDpad;
	/// stores  input device
	const DeviceInput* mp_devInput;
	/// stores id of pointer axis X
	unsigned int m_pointerAxisX;
	/// stores id of pointer axis Y
	unsigned int m_pointerAxisY;
	/// stores id of primary input button
	unsigned int m_pointerButtonId;
	/// stores id of "next" D-pad button
	unsigned int m_dpadIdNext;
	/// stores id of "previous" D-pad button
	unsigned int m_dpadIdPrev;
	/// stores id of enter D-pad button
	unsigned int m_dpadIdEnter;
	/// stores id of cancel D-pad button
	unsigned int m_dpadIdCancel;
	/// stores initialization data
	Var m_ini;
	
	/// stores styles
	std::map<std::string, WidgetStyle> mm_style;
	/// stores C++ function based Element2D factories
	std::map<std::string, Element2D* (*)(const Var &, const Canvas &)> mm_elementFactoryCFunc;
	/// stores Var based Element2D factories
	std::map<std::string, Var> mm_elementFactoryVar;
};

//--- class WidgetButton -------------------------------------------

/// a simple text or image button widget
class WidgetButton : public Widget {
public:
	/// constructor
	WidgetButton(const Canvas & root, const std::string & name = std::string() ) : Widget(root, name), mp_label(0), mp_img(0) {
		state(STATE_NORMAL); }
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;

	/// initializes element, calculates absolute coordinates, dimensions
	virtual void init();
	/// returns label
	virtual std::string text() const { return m_label; }
	/// sets label
	virtual bool text(const std::string & s) { m_label=s; if(mp_label) mp_label->text(m_label); return true; }
	/// sets image name
	virtual bool image(const std::string & img);
	/// sets state
	virtual void state(int st);

	/// sets width absolutely in pixels, if applicable
	virtual void width(float value) { Widget::width(value); }
	/// sets height absolutely in pixels, if applicable
	virtual void height(float value) { Widget::height(value); }
	/// returns width in pixels
	virtual float width() const;
	/// returns height in pixels
	virtual float height() const;
protected:
	/// stores label
	std::string m_label;
	/// pointer to label Element2D
	ElementString* mp_label;
	/// pointer to image Element2D
	ElementImg* mp_img;
};

//--- class WidgetPanel -------------------------------------------
/// container widget for manually arranging sub-widgets

class WidgetPanel : public Widget {
public:
	/// constructor
	WidgetPanel(const Canvas & root, const std::string & name = std::string() ) :
		Widget(root, name), mi_focus(mv_elem.end()), m_modal(false) { state(STATE_NORMAL); }
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
	/// returns true if the Widget is a modal widget
	/** modal Widgets automatically take the focus, any further input propagation is suppressed until the Widget is deleted */
	virtual bool modal() const { return m_modal; }
	/// makes this panel modal
	virtual void modal(bool yesno) { m_modal=yesno; }

	/// updates widget based on pointer input
	virtual int update(float pointerX, float pointerY, int buttonDown);
	/// appends an element or widget
	/** the lifetime of the appended Element2D is managed by the parent afterwards. Passed null pointers are ignored. */
	virtual Element2D * append(Element2D * pElem) { 
		pElem = ElementCanvas::append(pElem); if(pElem) mi_focus = mv_elem.end(); return pElem; }
		
	/// switches input focus to next subelement
	virtual Widget* next();
	/// switches input focus to previous subelement
	virtual Widget* previous();
	/// switches input focus one level down
	virtual Widget* enter();
	/// cleans up before switching input focus up
	virtual void leave();
		
protected:
	/// switches focus
	void switchFocus(Widget* pWidget);
	/// iterator pointing to currently focused child element
	std::vector<Element2D*>::iterator mi_focus;
	/// modal flag
	bool m_modal;
};


//--- class WidgetStack --------------------------------------------

/// container widget for automatically arranging sub-widgets horizontally or vertically
class WidgetStack : public WidgetPanel {
public:
	/// symbolic names for horizontal/vertical orientation
	enum {
		HORIZONTAL = 0,
		VERTICAL = 1,
	};
	/// converts an orientation string to the corresponding integer code
	static int orientationEncode(std::string s);
	/// converts an orientation code to the corresponding string
	static std::string orientationDecode(int code);
	
	/// constructor
	WidgetStack(const Canvas & root, int orientation=VERTICAL, const std::string & name = std::string() ) :
		WidgetPanel(root, name), m_vertical(orientation==VERTICAL) { state(STATE_NORMAL); }
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;		
	/// initializes element, calculates absolute coordinates, dimensions, arranges sub-elements
	virtual void init();
		
	/// returns orientation
	int orientation() const { return m_vertical ? VERTICAL : HORIZONTAL; }
protected:
	/// calculates dimensions
	std::pair<float, float> calcDim() const;
	/// stores direction, horizontal or vertical
	bool m_vertical;
};

//--- class WidgetList ---------------------------------------------

/// dynamic and paging stack widget
class WidgetList : public WidgetStack {
public:
	/// constructor
	WidgetList(const Canvas & root, int orientation=WidgetStack::VERTICAL, const std::string & name = std::string() ) :
		WidgetStack(root, orientation, name), m_first(0), m_pageSize(0) { }
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;
	/// initializes element, calculates absolute coordinates, dimensions, arranges sub-elements
	virtual void init();

	/// appends a single-line string menu entry
	void appendString(const std::string & label, const std::string & value);
	/// loads the content of a directory into the list
	void dir(const std::string & path="", const std::string & filter="*");
	/// loads an array Var into the list
	bool load(const Var & v);
	/// sets style for content
	void contentStyle(const std::string & s) { m_contentStyle=s; }
	
	/// sets page size, select 0 to disable paging
	void pageSize(unsigned int sz) { m_pageSize=sz; }
	/// returns page size, 0 means paging is disabled
	unsigned int pageSize() const { return m_pageSize; }
	/// switches to next page
	bool pageNext();
	/// switches to previous page
	bool pagePrev();
protected:
	/// updates displayed entries based on currently selected page
	void pageUpdate();
	/// stores index of currently displayed first entry
	unsigned int m_first;
	/// stores maximum number of simultaneously displayed entries on a single page
	unsigned int m_pageSize;
	/// entry buffer
	std::vector<std::pair<std::string,std::string> > mv_entry;
	/// name of entry style
	std::string m_contentStyle;
};

//--- class WidgetPopup --------------------------------------------

/// popup menu widget also serving as choice widget
class WidgetPopup : public WidgetButton {
public:
	/// constructor
	WidgetPopup(const Canvas & root, int orientation=WidgetStack::VERTICAL, const std::string & name = std::string() ) :
		WidgetButton(root, name), m_stack(root, orientation, name+"_substack"), m_open(false) { }
	/// destructor
	~WidgetPopup() { close(); }
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;

	/// updates widget based on pointer input
	virtual int update(float pointerX, float pointerY, int buttonDown);
	/// switches input focus to next subelement if applicable
	virtual Widget* next() { return (mv_elem.back()==&m_stack) ? m_stack.next() : WidgetButton::next(); }
	/// switches input focus to previous subelement if applicable
	virtual Widget* previous() { return (mv_elem.back()==&m_stack) ? m_stack.previous() : WidgetButton::previous(); }
	/// switches input focus one level down if applicable
	virtual Widget* enter();

	/// returns menu style name
	const std::string & contentStyle() const { return m_stack.style(); }
	/// sets menu style name
	virtual void contentStyle(const std::string & s) { m_stack.style(s); }
	/// appends an element or widget
	virtual Element2D * append(Element2D * pElem) { return ((pElem==mp_img)||(pElem==mp_label)) ? WidgetButton::append(pElem) : m_stack.append(pElem); }
	/// sets state
	virtual void state(int st);
protected:
	/// opens popup menu
	bool open();
	/// closes popup menu
	bool close();
	/// stack of children widgets
	WidgetStack m_stack;
	/// stores whether menu is open
	bool m_open;
};

//--- class WidgetInput --------------------------------------------

/// a single line text input widget
class WidgetInput : public Widget {
public:
	/// constructor
	WidgetInput(const Canvas & root, const std::string & name = std::string() ) : Widget(root, name), mp_label(0), m_tNow(0.0) {
		state(STATE_NORMAL); }
	/// returns type name
	virtual const char* type() const { return s_type; }
	/// defines type name
	static const char* s_type;

	/// initializes element, calculates absolute coordinates, dimensions
	virtual void init();
	/// returns command or code value associated with this widget
	virtual std::string value() const;
	/// sets command or code value associated with this widget
	void value(const std::string & s) { Widget::value(s); }
	/// returns text
	virtual std::string text() const { return m_text; }
	/// sets label
	virtual bool text(const std::string & s) { m_text=s; if(mp_label) mp_label->text(m_text); return true; }
	/// returns state
	int state() const { return Widget::state(); }
	/// sets state
	virtual void state(int st);
	/// updates widget based on time
	virtual void update(double deltaT);

protected:
	/// stores text
	std::string m_text;
	/// pointer to label Element2D
	ElementString* mp_label;
	/// stores current time, for caret animation
	double m_tNow;
};

#endif // _PRO_GUI_H
