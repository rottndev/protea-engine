#include "proGui.h"
#include <proIo.h>
#include <proStr.h>

#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif
#include <GL/gl.h>

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;

static int isPercent(const Var & v) {
	if(v.type()==Var::STRING) {
		string s = v.string();
		return ((s.size()>1)&&(s[s.size()-1]=='%')) ? 1 : 0;
	}
	else if(v.type()&Var::NUM) return 0;
	return -1;
}

static void toLower(string & s) {
    for(size_t i=0; i<s.size(); ++i)
        s[i]=static_cast<char>(tolower(s[i]));
}

//--- class WidgetStyle --------------------------------------------

WidgetStyle* WidgetStyle::sp_default = 0;

WidgetStyle::WidgetStyle() : mp_font(0), mp_parent(0) {
	for(size_t i=0; i<MEASURE_COUNTER; ++i)
		mv_measure[i]=-1.0f;
	for(size_t i=0; i<COLOR_COUNTER*Widget::STATE_COUNTER; ++i)
		mv_color[i]=Color::null;
}

const Font & WidgetStyle::font() const { 
	if(!mp_font&&mp_parent) 
		return mp_parent->font();
	if(!mp_font&&sp_default&&sp_default->mp_font) 
		return *sp_default->mp_font;
	return *mp_font; 
}

const Color & WidgetStyle::color(int type, int activityState) const {
	unsigned int colorIndex = type + activityState*WidgetStyle::COLOR_COUNTER;
	if((colorIndex<0)||(colorIndex >= COLOR_COUNTER*Widget::STATE_COUNTER)) return Color::null;
	if(mv_color[colorIndex]==Color::null) {
		if(mp_parent&&(this!=mp_parent)) return mp_parent->color(type,activityState);
		if(sp_default&&(this!=sp_default)) return sp_default->color(type,activityState);
	}
	return mv_color[colorIndex];
}

float WidgetStyle::measure(int type) const {
	if(type>=MEASURE_COUNTER) return 0.0f;
	if((mv_measure[type]==-1.0f)&&mp_parent) return mp_parent->measure(type);
	if((mv_measure[type]==-1.0f)&&sp_default) return sp_default->mv_measure[type];
	return mv_measure[type];
}

void WidgetStyle::defaultStyle(WidgetStyle & style) {
	sp_default = &style;
	if(!style.mp_font && DeviceWindow::instance() && DeviceWindow::instance()->font()) 
		style.font(*DeviceWindow::instance()->font());
	assert(style.mp_font!=0);
	if(style.mv_color[COLOR_BACKGR]==Color::null)
		style.mv_color[COLOR_BACKGR].value(1.0f, 1.0f, 1.0f, 0.7f);
	if(style.mv_color[COLOR_FOREGR]==Color::null)
		style.mv_color[COLOR_FOREGR].value(0.0f, 0.0f, 0.0f);
	if(style.mv_color[COLOR_BORDER]==Color::null)
		style.mv_color[COLOR_BORDER].value(0.0f, 0.0f, 0.0f);

	for(size_t i=0; i<MEASURE_COUNTER; ++i)
	if((i==PADDING) && (style.mv_measure[PADDING]==-1.0f))
		style.mv_measure[PADDING] = style.mp_font->height()*0.125f;
	else if(style.mv_measure[i]==-1.0f) style.mv_measure[i] = 0.0f;
}

static void readColors(const Var & v, Color* mv_color) {
	if((v["foregroundColor"].type()==Var::ARRAY) && (v["foregroundColor"].size()>2)) 
		v["foregroundColor"].numArray(mv_color[WidgetStyle::COLOR_FOREGR].value(), 4);
	if((v["backgroundColor"].type()==Var::ARRAY) && (v["backgroundColor"].size()>2)) 
		v["backgroundColor"].numArray(mv_color[WidgetStyle::COLOR_BACKGR].value(), 4);
	if((v["borderColor"].type()==Var::ARRAY) && (v["borderColor"].size()>2)) 
		v["borderColor"].numArray(mv_color[WidgetStyle::COLOR_BORDER].value(), 4);
}

bool WidgetStyle::interpret(const Var & v) {
	if(v.type()!=Var::MAP) return false;
	if(v["font"].type()) mp_font = DeviceWindow::instance()->font(v["font"].string());

	if(v["spacing"].type()) mv_measure[SPACING] = v["spacing"].number();
	if(v["padding"].type()) mv_measure[PADDING] = v["padding"].number();
	if(v["cornerRadius"].type()) mv_measure[CORNER_RADIUS] = v["cornerRadius"].number();
	if(v["borderWidth"].type()) mv_measure[BORDER_WIDTH] = v["borderWidth"].number();

	readColors(v, &mv_color[COLOR_COUNTER*Widget::STATE_NORMAL]);
	if(v["focus"].type()==Var::MAP) 
		readColors(v["focus"], &mv_color[COLOR_COUNTER*Widget::STATE_FOCUS]);		
	if(v["disabled"].type()==Var::MAP)
		readColors(v["disabled"], &mv_color[COLOR_COUNTER*Widget::STATE_DISABLED]);
	if(v["triggered"].type()==Var::MAP)
		readColors(v["triggered"], &mv_color[COLOR_COUNTER*Widget::STATE_TRIGGERED]);
	
	return true;
}

const WidgetStyle & WidgetStyle::defaultStyle() {
	assert(sp_default!=0);
	return *sp_default;
}

//--- Element2D Var adapter functions ------------------------------

static void Element2D_readVar(Element2D & elem, const Var & var) {
	int isPerc= isPercent(var["x"]);
	if(isPerc>0) elem.xRel(var["x"].number()*0.01f);
	else if(isPerc==0) elem.x(var["x"].number());

	isPerc= isPercent(var["y"]);
	if(isPerc>0) elem.yRel(var["y"].number()*0.01f);
	else if(isPerc==0) elem.y(var["y"].number());

	isPerc= isPercent(var["width"]);
	if(isPerc>0) elem.widthRel(var["width"].number()*0.01f);
	else if(isPerc==0) elem.width(var["width"].number());

	isPerc= isPercent(var["height"]);
	if(isPerc>0) elem.heightRel(var["height"].number()*0.01f);
	else if(isPerc==0) elem.height(var["height"].number());

	string s = var["alignX"].string();
	signed char alignX = (s=="center") ? Element2D::ALIGN_CENTER : (s=="right") ? Element2D::ALIGN_RIGHT : elem.align(false);
	s = var["alignY"].string();
	signed char alignY = (s=="center") ? Element2D::ALIGN_CENTER : (s=="top") ? Element2D::ALIGN_TOP : elem.align(true);
	elem.align(alignX, alignY);
	
	var["color"].numArray(elem.color().value(), 4);
	//printf("  color: %.2f %.2f %.2f %.2f\n", elem.color().r, elem.color().g, elem.color().b, elem.color().a);
	if(var["group"].type()) elem.group(var["group"].integer());	

	string text = var["text"].string();
	if(text.size()) elem.text(text);
}

static void ElementCanvas_readVar(ElementCanvas & canvas, const Var & v) {
	//cout << "ElementCanvas_readVar() \"" << canvas.name() << "\"... " << flush;
	Element2D_readVar(canvas, v);
	canvas.rotate(v["rotate"].number());
	float scale[2];
	scale[0] = scale[1] = v["scale"].number();
	if(scale[0]) canvas.scale(scale[0],scale[1]);
	else if(v["scale"].numArray(scale, 2)==2)
		canvas.scale(scale[0],scale[1]);

	const CanvasGui & root = dynamic_cast<const CanvasGui &>(canvas.root());
	if(v["content"].type()==Var::ARRAY) for(unsigned int i=0; i<v["content"].size(); ++i) {
		//cout << v["content"][i].string() << endl;
		canvas.append(root.createElement(v["content"][i]));		
	}
	//cout << "ElementCanvas_readVar() done." << endl;
}

static Element2D* ElementRect_create(const Var & v, const Canvas & root) {
	ElementRect* pElem = new ElementRect(root, v["name"].string());
	Element2D_readVar(*pElem, v);
	return pElem;
}

static Element2D* ElementLine_create(const Var & v, const Canvas & root) {
	ElementLine* pElem = new ElementLine(root, v["name"].string());
	if(v["lineWidth"].type()) pElem->lineWidth(v["lineWidth"].number());
	Element2D_readVar(*pElem, v);
	return pElem;
}

static Element2D* ElementString_create(const Var & v,  const Canvas & root) {
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(root);
	const Font* pFont = v["font"].type() ? root.font(v["font"].string()) : &gui.style(v["style"].string()).font();
	if(!pFont) return 0;
	ElementString* pElem = new ElementString(root, *pFont, v["name"].string());
	Element2D_readVar(*pElem, v);
	if(!v["color"].type())
		pElem->color(gui.style(v["style"].string()).color(WidgetStyle::COLOR_FOREGR, Widget::STATE_NORMAL)); 
	return pElem;
}

static Element2D* ElementText_create(const Var & v,  const Canvas & root) {
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(root);
	const Font* pFont = v["font"].type() ? root.font(v["font"].string()) : &gui.style(v["style"].string()).font();
	if(!pFont) return 0;
	ElementText* pElem = new ElementText(root, *pFont, v["name"].string());
	Element2D_readVar(*pElem, v);
	if(!v["color"].type())
		pElem->color(gui.style(v["style"].string()).color(WidgetStyle::COLOR_FOREGR, Widget::STATE_NORMAL)); 
	if(v["lineHeight"].type()) pElem->lineHeight(v["lineHeight"].number());
	string s = v["alignText"].string();
	if(s.size()) pElem->alignText( (s=="center") ? Element2D::ALIGN_CENTER : (s=="right") ? Element2D::ALIGN_RIGHT : Element2D::ALIGN_LEFT );
	return pElem;
}

static Element2D* ElementImg_create(const Var & v,  const Canvas & root) {
	unsigned int texId, width, height, depth;
	if(!root.textureProperties(v["resource"].string(), texId, width, height, depth)) return 0;
	//printf("%s %s %u %u %u %u\n", v["resource"].string().c_str(), v["name"].string().c_str(), texId, width, height, depth);
	ElementImg* pElem = new ElementImg(root, texId, width, height, depth, v["name"].string());
	Element2D_readVar(*pElem, v);
	return pElem;
}

static Element2D* ElementCanvas_create(const Var & v,  const Canvas & root) {
	if(v.type()!=Var::MAP) return 0;
	ElementCanvas * pElem = new ElementCanvas(root, v["name"].string());
	ElementCanvas_readVar(*pElem, v);
	return pElem;
}

static void Widget_readVar(Widget & w, const Var & v) {
	if(v["style"].type()) w.style(v["style"].string());
	if(v["value"].type()) w.value(v["value"].string());
	if(v["onEnter"].type()) w.event(Widget::EVT_ENTER, v["onEnter"].string());
	if(v["onCancel"].type()) w.event(Widget::EVT_CANCEL, v["onCancel"].string());
	if(v["onNext"].type()) w.event(Widget::EVT_NEXT, v["onNext"].string());
	if(v["onPrev"].type()) w.event(Widget::EVT_PREV, v["onPrev"].string());
}

static Element2D* WidgetButton_create(const Var & v,  const Canvas & root) {
	WidgetButton* pElem = new WidgetButton(root, v["name"].string());
	Element2D_readVar(*pElem, v);
	Widget_readVar(*pElem, v);
	if(v["image"].type()) pElem->image(v["image"].string());
	return pElem;
}

static void Panel_applyStyle(ElementPanel & panel, const std::string & styleName, int activityState = Widget::STATE_NORMAL) {
	const CanvasGui* pGui = dynamic_cast<const CanvasGui*>(&panel.root());
	if(!pGui) return;
	const WidgetStyle & style = pGui->style(styleName);
	panel.color(style.color(WidgetStyle::COLOR_BACKGR, activityState));
	panel.cornerRadius(style.measure(WidgetStyle::CORNER_RADIUS));
	if(style.measure(WidgetStyle::CORNER_RADIUS)) panel.corners(ElementPanel::CORNER_ALL);
	panel.borderWidth(style.measure(WidgetStyle::BORDER_WIDTH));
	panel.borderColor(style.color(WidgetStyle::COLOR_BORDER, activityState));
}

static void Panel_readVar(ElementPanel & panel, const Var & v) {
	//cout << "Panel_readVar()... " << flush;
	// read style definition:
	if(v["style"].type()) Panel_applyStyle(panel, v["style"].string());
	// overwrite style by individual definitions:
	if(v["color"].type()) v["color"].numArray(panel.color().value(), 4);
	if(v["cornerRadius"].type()) panel.cornerRadius(v["cornerRadius"].number());
	if(v["corners"].type()) {
		if((v["corners"].type()==Var::ARRAY)&&(v["corners"].size()>3)) {
			unsigned int c = v["corners"][0].number() ? ElementPanel::CORNER_LEFT_BOTTOM : ElementPanel::CORNER_NONE;
			if(v["corners"][1].number()) c+= ElementPanel::CORNER_RIGHT_BOTTOM;
			if(v["corners"][2].number()) c+= ElementPanel::CORNER_RIGHT_TOP;
			if(v["corners"][3].number()) c+= ElementPanel::CORNER_LEFT_TOP;
			panel.corners(c);
		}
		else panel.corners(v["corners"].number() ? ElementPanel::CORNER_ALL : ElementPanel::CORNER_NONE);
	}
	if(v["borderWidth"].type()) panel.borderWidth(v["borderWidth"].number());
	if(v["borderColor"].type()) v["borderColor"].numArray(panel.borderColor().value(), 4);
	//cout << "Panel_readVar() done.\n" << flush;
}

static Element2D* WidgetPanel_create(const Var & v,  const Canvas & root) {
	if(v.type()!=Var::MAP) return 0;
	WidgetPanel * pElem = new WidgetPanel(root, v["name"].string());
	ElementCanvas_readVar(*pElem, v);
	Widget_readVar(*pElem, v);
	pElem->modal(v["modal"].boolean());
	Panel_readVar(*dynamic_cast<ElementPanel*>(pElem->background()), v);
	return pElem;
}

static Element2D* WidgetStack_create(const Var & v,  const Canvas & root) {
	int orientation = (v["orientation"].type()==Var::STRING) ? WidgetStack::orientationEncode(v["orientation"].string()) : WidgetStack::VERTICAL;
	WidgetStack* pElem = new WidgetStack(root, orientation, v["name"].string());
	ElementCanvas_readVar(*pElem, v);
	Widget_readVar(*pElem, v);
	pElem->modal(v["modal"].boolean());
	Panel_readVar(*dynamic_cast<ElementPanel*>(pElem->background()), v);
	return pElem;
}

static Element2D* WidgetPopup_create(const Var & v,  const Canvas & root) {
	int orientation = (v["orientation"].type()==Var::STRING) ? WidgetStack::orientationEncode(v["orientation"].string()) : WidgetStack::VERTICAL;
	WidgetPopup* pElem = new WidgetPopup(root, orientation, v["name"].string());
	ElementCanvas_readVar(*pElem, v);
	Widget_readVar(*pElem, v);
	if(v["image"].type()) pElem->image(v["image"].string());
	if(v["contentStyle"].type()) pElem->contentStyle(v["contentStyle"].string());
	return pElem;
}

static Element2D* WidgetSeparator_create(const Var & v,  const Canvas & root) {
	ElementRect* pElem = new ElementRect(root, v["name"].string());
	if(v["spacing"].type()&Var::NUM) {
		pElem->width(v["spacing"].number());
		pElem->height(v["spacing"].number());
	}
	else {
		pElem->width(1.0f);
		pElem->height(1.0f);
	}
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(root);
	pElem->color(gui.style(v["style"].string()).color(WidgetStyle::COLOR_FOREGR, Widget::STATE_NORMAL)); 
	return pElem;
}

static Element2D* WidgetList_create(const Var & v,  const Canvas & root) {
	int orientation = (v["orientation"].type()==Var::STRING) ? WidgetStack::orientationEncode(v["orientation"].string()) : WidgetStack::VERTICAL;	
	WidgetList* pElem = new WidgetList(root, orientation, v["name"].string());
	ElementCanvas_readVar(*pElem, v);
	Widget_readVar(*pElem, v);
	if(v["contentStyle"].type()) pElem->contentStyle(v["contentStyle"].string());
	if(v["pageSize"].type()) pElem->pageSize(v["pageSize"].integer());
	if(v["path"].type()) pElem->dir(v["path"].string(), v["filter"].string());
	return pElem;
}

static Element2D* WidgetInput_create(const Var & v,  const Canvas & root) {
	WidgetInput* pElem = new WidgetInput(root, v["name"].string());
	Element2D_readVar(*pElem, v);
	Widget_readVar(*pElem, v);
	if(v["text"].type()) pElem->text(v["text"].string());
	return pElem;
}

//--- class CanvasGui ----------------------------------------------

CanvasGui::CanvasGui(DeviceWindow & window, const DeviceInput & devInput, const Var & ini) : 
	Canvas(window), Callable(), m_state(0), mp_focus(0), mp_modal(0), mp_textInput(0), mi_elem(mv_elem.end()), m_inputDpad(false), mp_devInput(&devInput), 
	m_pointerAxisX(3), m_pointerAxisY(4), m_pointerButtonId(0),
	m_dpadIdNext(5), m_dpadIdPrev(4), m_dpadIdEnter(6), m_dpadIdCancel(7), m_ini(ini) { 

	registerElementFactory(ElementRect_create, ElementRect::s_type);
	registerElementFactory(ElementImg_create, ElementImg::s_type);
	registerElementFactory(ElementString_create, ElementString::s_type);
	registerElementFactory(ElementText_create, ElementText::s_type);
	registerElementFactory(ElementLine_create, ElementLine::s_type);
	registerElementFactory(ElementCanvas_create, ElementCanvas::s_type);
	registerElementFactory(WidgetButton_create, WidgetButton::s_type);
	registerElementFactory(WidgetPanel_create, WidgetPanel::s_type);
	registerElementFactory(WidgetStack_create, WidgetStack::s_type);
	registerElementFactory(WidgetPopup_create, WidgetPopup::s_type);
	registerElementFactory(WidgetSeparator_create, "separator");
	registerElementFactory(WidgetList_create, WidgetList::s_type);
	registerElementFactory(WidgetInput_create, WidgetInput::s_type);
		
	initResources();
	registerElementFactories(ini["elementTemplates"]);	
		
	// parse style data:
	Var keys = ini["styles"].info();
	for(unsigned int i=0; i<keys.size(); ++i) {
		string name = keys[i].string();
		mm_style.insert(make_pair(name, WidgetStyle()));
		mm_style[name].interpret(ini["styles"][name]);
	}
	// set default style:
	if(mm_style.find("default")!=mm_style.end()) WidgetStyle::defaultStyle(mm_style["default"]);
	else {
		if(mm_style.begin()==mm_style.end()) mm_style.insert(make_pair("default", WidgetStyle()));
		WidgetStyle::defaultStyle(mm_style.begin()->second);
	}
	// set parent styles in a second iteration, when all styles are available:
	for(unsigned int i=0; i<keys.size(); ++i) {
		string name = keys[i].string();
		if(!ini["styles"][name]["extends"].type()) continue;
		string parentName = ini["styles"][name]["extends"].string();
		if(mm_style.find(parentName)!=mm_style.end()) mm_style[name].parent(&mm_style.find(parentName)->second);
	}
	
	// parse DPad navigation settings:
	if(ini["navigation"].type()==Var::MAP) {
		Var navIni = ini["navigation"];
		if(navIni["enter"].type()) m_dpadIdEnter = navIni["enter"].integer();
		if(navIni["cancel"].type()) m_dpadIdCancel = navIni["cancel"].integer();
		if(navIni["next"].type()) m_dpadIdNext = navIni["next"].integer();
		if(navIni["prev"].type()) m_dpadIdPrev = navIni["prev"].integer();
	}
}

void CanvasGui::registerElementFactories(const Var & v) {
	if(v.type()!=Var::MAP) return;
	// parse element factory data:
	Var keys = v.info();
	for(unsigned int i=0; i<keys.size(); ++i) {
		string name = keys[i].string();
		if(v[name].type()==Var::MAP) registerElementFactory(v[name], name);
	}
}

Var CanvasGui::call(const std::string & cmd, const Var & arg) {
	if(cmd=="append") return append(arg[0])!=0;
	if(cmd=="erase") return erase(arg[0].string());
	if(cmd=="clear") clear();
	else if(cmd=="pageNext") {
		WidgetList* pList = dynamic_cast<WidgetList*>(find(arg[0].string()));
		if(!pList) return false;
		return pList->pageNext();
	}
	else if(cmd=="pagePrev") {
		WidgetList* pList = dynamic_cast<WidgetList*>(find(arg[0].string()));
		if(!pList) return false;
		return pList->pagePrev();
	}
	else if(cmd=="focus") {
		if(!arg[0].type()) return mp_focus ? mp_focus->name() : Var::null;
		else if(arg[0].type()==Var::STRING) return (focus(arg[0].string())!=0);
	}
	else if(cmd=="value") {
		Widget* pWidget = dynamic_cast<Widget*>(find(arg[0].string()));
		if(!pWidget) return false;
		if(arg[1].type()) pWidget->value(arg[1].string());
		return pWidget->value();
	}
	else if(cmd=="text") {
		Element2D* pElem =find(arg[0].string());
		if(!pElem) return false;
		if(arg[1].type()) pElem->text(arg[1].string());
		return pElem->text();
	}
	else if(cmd=="load") {
		WidgetList* pList = dynamic_cast<WidgetList*>(find(arg[0].string()));
		if(!pList||(arg[1].type()!=Var::ARRAY)) return false;
		return pList->load(arg[1]);
	}
	else if(cmd=="inputByPointer") {
		if(arg[0].type()) inputByPointer(arg[0].boolean());
		return inputByPointer();
	}
	return Var::null;
}

void CanvasGui::textInput(WidgetInput* pInput) {
	mp_textInput=pInput;
	if(mp_textInput) m_wnd.textInput(mp_textInput->text().c_str());
}

int CanvasGui::update(double deltaT) {
	m_value.clear();
	if(m_state == Widget::STATE_TRIGGERED) m_state=Widget::STATE_NORMAL;
	
	// update children by time:
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
		Widget* pWidget = dynamic_cast<Widget*>(*it);
		if(pWidget) pWidget->update(deltaT);
	}
	// propagate text input:
	if(mp_textInput&&(mp_textInput->state()==Widget::STATE_FOCUS)) {
		if(mp_textInput->text()!=m_wnd.textInput()) mp_textInput->text(m_wnd.textInput());
		m_state = Widget::STATE_FOCUS;
		if(m_wnd.textComplete() && mp_textInput->text().size()) {
			m_value = mp_textInput->value();
			if(m_value.size()) {
				m_state = Widget::STATE_TRIGGERED;
				return m_state;
			}
		}
	}
	
	if(!mp_devInput) return Widget::STATE_DISABLED;
	if(mp_modal&&!mp_focus) mp_focus = mp_modal;
	return m_inputDpad ? updateByDpad() : updateByPointer();
}


int CanvasGui::updateByDpad() {
	if(!mp_modal && (m_state < Widget::STATE_FOCUS)) return m_state;

	if(mp_devInput->buttonDown(m_dpadIdCancel)) {
		//cout << "[Cancel]" << endl; if(mp_focus) cout << "  " << mp_focus->name() << endl;
		if(!mp_focus) {
			if(mi_elem!=mv_elem.end())
				dynamic_cast<Widget*>(*mi_elem)->state(Widget::STATE_NORMAL);
			m_state = Widget::STATE_NORMAL;
			return m_state;
		}
	
		if(mp_focus->event(Widget::EVT_CANCEL).size()) {
			m_value = mp_focus->event(Widget::EVT_CANCEL);
			m_state = Widget::STATE_TRIGGERED;
		}
		else {
			mp_focus->leave();
			mp_focus = 0;
			m_state = Widget::STATE_NORMAL;
		}
	}

	else if(mp_devInput->buttonDown(m_dpadIdEnter)) {
		//cout << "[Enter]" << endl;
		if(focus()&&focus()->event(Widget::EVT_ENTER).size()) {
			m_value = focus()->event(Widget::EVT_ENTER);
			m_state = Widget::STATE_TRIGGERED;
			return m_state;
		}
		if(mp_focus) {
			Widget* pWidget = mp_focus->enter();
			if(pWidget&&(pWidget!=mp_focus)) {
				if(mp_focus) mp_focus->state(Widget::STATE_NORMAL);
				mp_focus = pWidget;
				mp_focus->state(Widget::STATE_FOCUS);
				textInput(dynamic_cast<WidgetInput*>(mp_focus));
				m_state = Widget::STATE_FOCUS;
			}
			else if(mp_focus->value().size()) { // value of current focus widget is selected
				m_value = mp_focus->value();
				m_state = Widget::STATE_TRIGGERED;
				mp_focus->state(Widget::STATE_NORMAL);
				mp_focus = (mp_focus->parent()==this) ? 0 : dynamic_cast<Widget*>(mp_focus->parent());
			}
			//if(mp_focus) cout << "  " << mp_focus->name() << endl;
		}
		else if(mv_elem.size()) { // focus on / evaluate current sub-element
			if(mi_elem>=mv_elem.end()) mi_elem=mv_elem.begin();
			Widget* pWidget = dynamic_cast<Widget*>(*mi_elem);
			while(!pWidget && (mi_elem<mv_elem.end())) {
				pWidget= dynamic_cast<Widget*>(*mi_elem);
				if(!pWidget) ++mi_elem;
			}
			if(!pWidget) return m_state;
				
			mp_focus = pWidget->enter();
			if(mp_focus||(pWidget->state()<Widget::STATE_FOCUS)) {
				pWidget->state(Widget::STATE_FOCUS);
				textInput(dynamic_cast<WidgetInput*>(pWidget));
				m_state = Widget::STATE_FOCUS;
			}
			else if(pWidget->value().size()) { // value of current sub-element is selected
				m_value = pWidget->value();
				m_state = Widget::STATE_TRIGGERED;
				pWidget->state(Widget::STATE_NORMAL);
			}
			//if(mp_focus) cout << "  " << mp_focus->name() << endl;
		}
	}

	else if(mp_devInput->buttonDown(m_dpadIdNext)) {
		//cout << "[Next] " << endl; if(mp_focus) cout << "  " << mp_focus->name() << endl;
		if(mp_focus) {
			if(mp_focus->event(Widget::EVT_NEXT).size()) {
				m_value = mp_focus->event(Widget::EVT_NEXT);
				m_state = Widget::STATE_TRIGGERED;
			}
			else if(mp_focus->next()) m_state = Widget::STATE_FOCUS;
			else {
				mp_focus->state(Widget::STATE_NORMAL);
				mp_focus = dynamic_cast<Widget*>(mp_focus->parent());
				if(mp_focus) {
					mp_focus->state(Widget::STATE_FOCUS);
					textInput(dynamic_cast<WidgetInput*>(mp_focus));
				}
			}
		}
		if(!mp_focus&&mv_elem.size()) {
			bool navAllowed = mp_modal ? false : true;
			if(navAllowed&&(mi_elem!=mv_elem.end())) {
				Widget* pWidget = dynamic_cast<Widget*>(*mi_elem);
				if(pWidget) navAllowed = pWidget->modal() ? false : true;
			}
			if(navAllowed) for(vector<Element2D*>::iterator it = mi_elem+1 ; it!=mi_elem; ++it) {
				if(it>=mv_elem.end()) it=mv_elem.begin();
				Widget* pWidget = dynamic_cast<Widget*>(*it);
				if(!pWidget) continue;
				// switch focus:
				//cout << "  " << pWidget->type() << " " << pWidget->name() << endl;
				if(mi_elem!=mv_elem.end())
					dynamic_cast<Widget*>(*mi_elem)->state(Widget::STATE_NORMAL);
				mi_elem = it;
				pWidget->state(Widget::STATE_FOCUS);
				textInput(dynamic_cast<WidgetInput*>(pWidget));
				m_state = Widget::STATE_FOCUS;
				break;
			}
		}
	}

	else if(mp_devInput->buttonDown(m_dpadIdPrev)) {
		//cout << "[Prev]" << endl; if(mp_focus) cout << "  " << mp_focus->name() << endl;
		if(mp_focus) {
			if(mp_focus->event(Widget::EVT_PREV).size()) {
				m_value = mp_focus->event(Widget::EVT_PREV);
				m_state = Widget::STATE_TRIGGERED;
			}
			else if(mp_focus->previous()) m_state = Widget::STATE_FOCUS;
			else {
				mp_focus->state(Widget::STATE_NORMAL);
				mp_focus = dynamic_cast<Widget*>(mp_focus->parent());
				if(mp_focus) {
					mp_focus->state(Widget::STATE_FOCUS);
					textInput(dynamic_cast<WidgetInput*>(mp_focus));
				}
			}
		}
		if(!mp_focus&&mv_elem.size()) {
			bool navAllowed = mp_modal ? false : true;
			if(navAllowed&&(mi_elem!=mv_elem.end())) {
				Widget* pWidget = dynamic_cast<Widget*>(*mi_elem);
				if(pWidget) navAllowed = pWidget->modal() ? false : true;
			}
			if(navAllowed) for(vector<Element2D*>::iterator it = (mi_elem == mv_elem.begin()) ? mv_elem.end()-1 : mi_elem-1 ; it!=mi_elem; --it) {
				if(it<mv_elem.begin()) it=mv_elem.end()-1;
				Widget* pWidget = dynamic_cast<Widget*>(*it);
				if(!pWidget) continue;
				// switch focus:
				//cout << "  " << pWidget->type() << " " << pWidget->name() << endl;
				if(mi_elem!=mv_elem.end())
					dynamic_cast<Widget*>(*mi_elem)->state(Widget::STATE_NORMAL);
				mi_elem = it;
				pWidget->state(Widget::STATE_FOCUS);
				textInput(dynamic_cast<WidgetInput*>(pWidget));
				m_state = Widget::STATE_FOCUS;
				break;
			}
		}
	}
	return m_state;
}


int CanvasGui::updateByPointer() {
	float pointerX=0.5f*(mp_devInput->axis(m_pointerAxisX)+1.0f)*m_wnd.width();
	float pointerY=0.5f*(mp_devInput->axis(m_pointerAxisY)+1.0f)*m_wnd.height();
	int buttonDown = mp_devInput->buttonDown(m_pointerButtonId);

	Widget* pFocus = 0;
	if((mi_elem!=mv_elem.end())||mp_modal) {
		Widget* pWidget = mp_modal ? mp_modal : dynamic_cast<Widget*>(*mi_elem);
		if(pWidget) {
			int res = pWidget->update(pointerX,pointerY, buttonDown);
			if(res>Widget::STATE_NORMAL) pFocus = pWidget;
		}
		if(!pFocus && pWidget->modal()) pFocus = pWidget;
	}				
	if(!pFocus) for(vector<Element2D*>::reverse_iterator it=mv_elem.rbegin(); it!=mv_elem.rend(); ++it) {
		Widget* pWidget = dynamic_cast<Widget*>(*it);
		if(!pWidget) continue;
		int res = pWidget->update(pointerX,pointerY, buttonDown);
		if(res>Widget::STATE_NORMAL) {
			pFocus = pWidget;			
			break;
		}
	}
	
	if(mp_focus!=pFocus) {
		if(mp_focus) mp_focus->state(Widget::STATE_NORMAL);
		mp_focus = pFocus;
		textInput(dynamic_cast<WidgetInput*>(mp_focus));
	}
	if(mp_focus) {
		m_value = mp_focus->value();
		m_state = (buttonDown && m_value.size()) ? 
			Widget::STATE_TRIGGERED : Widget::STATE_FOCUS;
	}
	else m_state = Widget::STATE_NORMAL;
	return m_state;
}


void CanvasGui::draw() {
	if(mp_cursor) {
		mp_cursor->x(0.5f*(mp_devInput->axis(m_pointerAxisX)+1.0f)*m_wnd.width());
		mp_cursor->y(0.5f*(mp_devInput->axis(m_pointerAxisY)+1.0f)*m_wnd.height());
		mp_cursor->init();
	}
	Canvas::draw();
}

const Widget* CanvasGui::focus() const { 
	if(mp_focus) return mp_focus; 
	if((mi_elem<mv_elem.end())&&m_inputDpad)
		return dynamic_cast<const Widget*>(*mi_elem);
	return 0;
}

const Widget* CanvasGui::focus(const std::string & name) { 
	if(name.empty()) { // focus off
		if(mp_focus) {
			mp_focus->state(Widget::STATE_NORMAL);
			mp_focus = 0;
		}
		else if(mi_elem<mv_elem.end()) {
			Widget* pWidget = dynamic_cast<Widget*>(*mi_elem);
			if(pWidget) pWidget->state(Widget::STATE_NORMAL);
		}
		m_state = Widget::STATE_NORMAL;
		return 0;
	}
	Widget* pWidget = dynamic_cast<Widget*>(find(name));
	if(!pWidget) return 0;
	if(mp_focus&&(mp_focus!=pWidget)) 
		mp_focus->state(Widget::STATE_NORMAL);
	mp_focus = pWidget;
	mp_focus->state(Widget::STATE_FOCUS);
	mp_focus->enter();
	textInput(dynamic_cast<WidgetInput*>(mp_focus));
	if(m_inputDpad) 
		m_state = Widget::STATE_FOCUS;
	return mp_focus;
}

/// replace all occurences of a search string within a Var object
static void replaceAll(Var & v, const std::string & search, const std::string & repl) {
	switch(v.type()) {
	case Var::STRING:
		v = replaceAll(v.string(), search, repl);
		break;
	case Var::MAP: {
		Var keys = v.info();
		for(unsigned int i=0; i<keys.size(); ++i) replaceAll(v[keys[i].string()], search, repl);
		break; }
	case Var::ARRAY:
		for(unsigned int i=0; i<v.size(); ++i) replaceAll(v[i], search, repl);
		break;
	}
}

Element2D* CanvasGui::createElement(const Var & v) const {
	//cout << "CanvasGui::createElement(" << v.string() << ")" << endl;
	if(v.type()!=Var::MAP) return 0;
	string type = v["element"].string();
	if(!type.size()) return 0;
	map<string, Element2D* (*)(const Var &, const Canvas &)>::const_iterator it = mm_elementFactoryCFunc.find(type);
	if(it!=mm_elementFactoryCFunc.end()) return (*(it->second))(v, *this);
	
	map<string, Var>::const_iterator itVar = mm_elementFactoryVar.find(type);
	if(itVar!=mm_elementFactoryVar.end()) {
		Var output = itVar->second;
		Var params = output["params"];
		output.erase("params");
		if(params.type()==Var::ARRAY) for(unsigned int i=0; i<params.size(); ++i) { // replace parameters
			string param = params[i].string();
			replaceAll(output, '$'+param+'$', v[param].string());
		}
		return createElement(output);
	}
	return 0;
}

Element2D * CanvasGui::append(Element2D * pElem) { 
	pElem = ElementCanvas::append(pElem); 
	if(pElem) {
		if(mp_focus) {
			mp_focus->state(Widget::STATE_NORMAL);
			mp_focus=0;
		}
		Widget* pWidget = dynamic_cast<Widget*>(pElem);
		if(pWidget&&pWidget->modal()) {
			mp_modal = pWidget;
			mi_elem =  mv_elem.end()-1;
		}
		else mi_elem = mv_elem.end();
	}
	return pElem; 
}

Element2D* CanvasGui::append(const Var & v) {
	//cout << "CanvasGui::append " << v.string() << endl;
	Element2D* ret=0;
	if(v.type()==Var::ARRAY) for(unsigned int i=0; i<v.size(); ++i) {
		//dout("Appending "+v[i].string()+' ');
		Element2D* pElem = createElement(v[i]);
		if(!pElem) continue;
		append(pElem);
		pElem->init();
		//dout("done.\n");
		ret = pElem;
	}
	else if(v.type()==Var::MAP)  {
		ret = createElement(v);
		if(!ret) return false;
		append(ret);
		ret->init();
	}
	return ret;
}

bool CanvasGui::erase(Element2D* pElem) { 
	if(Canvas::erase(pElem)) { 
		mi_elem = mv_elem.end(); 
		mp_focus=0; 
		mp_modal=0; 
		mp_textInput=0;
		return true; 
	} 
	return false; 
}

Element2D* CanvasGui::detach(Element2D* pElem) { 
	pElem = Canvas::detach(pElem);
	if(pElem) {
 		mi_elem = mv_elem.end(); 
		mp_focus=0; 
		mp_modal=0; 
		mp_textInput=0;
	}
	return pElem;
}

void CanvasGui::clear() { 
	Canvas::clear(); 
	mp_focus=0; 
	mp_modal=0;
	mp_textInput=0;
	m_value.clear(); 
	mi_elem=mv_elem.end(); 
}

void CanvasGui::initResources() {
	//dout("initializing resources...");
	if(m_ini["resources"]["fonts"].type()==Var::MAP) {
		//dout(" fonts...");
		Var keys = m_ini["resources"]["fonts"].info();
		for(unsigned int i=0; i<keys.size(); ++i) {
			string key = keys[i].string();
			Image* pFontImg = TextureMgr::singleton().load(m_ini["resources"]["fonts"][key]["image"].string());
			if(!pFontImg) continue;
			m_wnd.createFont(*pFontImg, key);
			delete pFontImg;
		}
	}
	if(m_ini["resources"]["images"].type()==Var::MAP) {
		//dout(" images...");
		Var keys = m_ini["resources"]["images"].info();
		for(unsigned int i=0; i<keys.size(); ++i) {
			string key = keys[i].string();
			Image* pImg = TextureMgr::singleton().load(m_ini["resources"]["images"][key]["image"].string());
			if(!pImg) continue;
			m_wnd.createTexture(*pImg, key);
			delete pImg;
		}
	}
	//dout(" done.\n");
}

void CanvasGui::inputPointerSource(const DeviceInput & source, unsigned int axisX, unsigned int axisY, unsigned int buttonId) {
	mp_devInput = &source;
	m_inputDpad = false;
	m_pointerAxisX=axisX;
	m_pointerAxisY=axisY;
	m_pointerButtonId = buttonId;
}

void CanvasGui::inputDpadSource(const DeviceInput & source, unsigned int buttonIdNext, unsigned int buttonIdPrev, unsigned int buttonIdOk, unsigned int buttonIdCancel) {
	mp_devInput = &source;
	m_inputDpad = true;
	m_dpadIdNext = buttonIdNext;
	m_dpadIdPrev = buttonIdPrev;
	m_dpadIdEnter = buttonIdOk;
	m_dpadIdCancel = buttonIdCancel;
}

const WidgetStyle & CanvasGui::style(const std::string & name) const {
	map<string, WidgetStyle>::const_iterator it = mm_style.find(name);
	return (it==mm_style.end()) ? WidgetStyle::defaultStyle() : it->second;
}


//--- class Widget -------------------------------------------------

Widget::Widget(const Canvas & root, const std::string & name) : 
	ElementCanvas(root, name), m_state(STATE_DISABLED), mp_background(0) {	
	ElementPanel* pPanel = new ElementPanel(root, m_name+"_panel");
	Panel_applyStyle(*pPanel, m_style);
	background(pPanel); 
}

void Widget::init() {
	ElementCanvas::init();
	if(mp_background) {
		mp_background->widthRel(1.0f);
		mp_background->heightRel(1.0f);
		mp_background->init();
	}
}

void Widget::draw() {
	glPushMatrix();
	float posX = x(), posY= y();
	glTranslatef(posX, posY, 0.0f);
	glRotatef(m_rot, 0.0f, 0.0f,1.0f);
	glScalef(m_scale[0],m_scale[1],1.0f);
	
	glTranslatef(-posX, -posY, 0.0f);
	if(!mp_background) ElementRect::draw();
	
	glTranslatef(m_bounding[0], m_bounding[1], 0.0f);
	if(mp_background) mp_background->draw();
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it)
		(*it)->draw();
	glPopMatrix();
}

int Widget::update(float pointerX, float pointerY, int buttonDown) { 
	if(m_state==STATE_DISABLED) return m_state;
	int state = STATE_NORMAL;
	if((pointerX>=m_bounding[0])&&(pointerY>=m_bounding[1])&&(pointerX<m_bounding[2])&&(pointerY<m_bounding[3]))
		state = STATE_FOCUS;
	if(state!=m_state) this->state(state);
	return m_state; 
}

void Widget::update(double deltaT) {
	Widget* pWidget = dynamic_cast<Widget*>(mp_background);
	if(pWidget) pWidget->update(deltaT);

	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
		pWidget = dynamic_cast<Widget*>(*it);
		if(pWidget) pWidget->update(deltaT);
	}
}

void Widget::style(const std::string & s) { 
	m_style=s; 
	if(mp_background) {
		const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
		ElementPanel * pPanel = dynamic_cast<ElementPanel*>(mp_background);
		if(pPanel) Panel_applyStyle(*pPanel, m_style);
		else mp_background->color(gui.style(m_style).color(WidgetStyle::COLOR_BACKGR));
	}
}

//--- class WidgetButton -------------------------------------------

const char* WidgetButton::s_type = "button";

void WidgetButton::init() {
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
	color(gui.style(m_style).color(WidgetStyle::COLOR_BACKGR, m_state)); 	
	if(!mp_label && !mp_img && m_label.size()) {
		mp_label = new ElementString(gui, gui.style(m_style).font(), "label");
		mp_label->xRel(0.5f);
		mp_label->yRel(0.5f);
		mp_label->widthRel(1.0f);
		mp_label->heightRel(1.0f);
		mp_label->align(Element2D::ALIGN_CENTER, Element2D::ALIGN_CENTER);
		mp_label->text(m_label);
		mp_label->color(gui.style(m_style).color(WidgetStyle::COLOR_FOREGR, m_state)); 
		append(mp_label);
	}
	if(!m_dim[0] && mp_label) width(width());
	if(!m_dim[1] && mp_label) height(height());
	Widget::init();
}

void WidgetButton::state(int st) { 
	if(st==m_state) return; 
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
	const WidgetStyle & style = gui.style(m_style);
	if(mp_label) mp_label->color(style.color(WidgetStyle::COLOR_FOREGR, st)); 
	if(mp_img&&(mp_img->textureDepth()==1)) 
		mp_img->color(style.color(WidgetStyle::COLOR_FOREGR, st));
	if(mp_background) {
		ElementPanel * pPanel = dynamic_cast<ElementPanel*>(mp_background);
		if(pPanel) Panel_applyStyle(*pPanel, m_style, st);
		else mp_background->color(style.color(WidgetStyle::COLOR_BACKGR, st));
	}
	else color(style.color(WidgetStyle::COLOR_BACKGR, st));
	Widget::state(st);
}

bool WidgetButton::image(const std::string & img) { 
	unsigned int id, w, h, d;
	if(!m_root.textureProperties(img, id, w, h, d)) return false;
	if(mp_label) {
		erase(mp_label);
		mp_label = 0;
	}
	if(mp_img) erase(mp_img);
	mp_img = new ElementImg(m_root, id, w, h, d);
	append(mp_img);
	if(mp_img->textureDepth()==1) {
		const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
		mp_img->color(gui.style(m_style).color(WidgetStyle::COLOR_FOREGR, m_state)); 
	}
	if(!m_dim[0]) width((float)w);
	else mp_img->width(width());
	if(!m_dim[1]) height((float)h);
	else mp_img->height(height());
	return true;
}

float WidgetButton::width() const { 
	if(m_dim[0]) return Widget::width();
	const WidgetStyle& style = dynamic_cast<const CanvasGui&>(m_root).style(m_style);
	float w = 2*style.measure(WidgetStyle::PADDING);
	return m_label.size() ? (style.font().width(m_label)+w) : mp_img ? mp_img->width()+w : w; 
}

float WidgetButton::height() const { 
	if(m_dim[1]) return Widget::height();
	const WidgetStyle& style = dynamic_cast<const CanvasGui&>(m_root).style(m_style);
	float h = 2*style.measure(WidgetStyle::PADDING);
	return m_label.size() ? (style.font().height()+h) : mp_img ? mp_img->height()+h : h; 
}

//--- class WidgetPanel -------------------------------------------

const char* WidgetPanel::s_type = "panel";

void WidgetPanel::switchFocus(Widget* pWidget) {
	if(!pWidget) return;
	//cout << "  " << m_name <<" WidgetPanel::switchFocus() to " << pWidget->name() << endl;
	pWidget->state(Widget::STATE_FOCUS);
	m_value = pWidget->value();
	WidgetInput* pInput = dynamic_cast<WidgetInput*>(pWidget);
	if(pInput) const_cast<CanvasGui&>(dynamic_cast<const CanvasGui&>(m_root)).textInput(pInput);
}

int WidgetPanel::update(float pointerX, float pointerY, int buttonDown) { 	
	m_value.clear();
	Widget::update(pointerX, pointerY, buttonDown);

	toLocal(pointerX, pointerY);
	for(vector<Element2D*>::reverse_iterator it=mv_elem.rbegin(); it!=mv_elem.rend(); ++it) {
		Widget* pWidget = dynamic_cast<Widget*>(*it);
		if(!pWidget) continue;
		int res = pWidget->update(pointerX,pointerY, buttonDown);
		if(res>Widget::STATE_NORMAL) {
			m_state = res;
			switchFocus(pWidget);
		}
		else pWidget->state(Widget::STATE_NORMAL);
	}
	return m_state; 
}

Widget* WidgetPanel::enter() { 
	//cout << "  WidgetPanel::enter() " << m_name << endl;
	if(mi_focus<mv_elem.end()) { // propagate to child:
		Widget* pWidget = dynamic_cast<Widget*>(*mi_focus);
		if(!pWidget) return 0;
		if(pWidget->state()==Widget::STATE_FOCUS) return pWidget->enter();
		switchFocus(pWidget);
		return this;
	}
	// focus on first sub-element
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
		Widget* pWidget = dynamic_cast<Widget*>(*it);
		if(!pWidget) continue;
		mi_focus = it;
		switchFocus(pWidget);
		return this;
	}
	return 0; // no sub-element found
}

void WidgetPanel::leave() {
	if(mi_focus!=mv_elem.end()) { // unfocus:
		Widget* pWidget = dynamic_cast<Widget*>(*mi_focus);
		if(pWidget) pWidget->leave();
	}
	m_value.clear();
	state(Widget::STATE_NORMAL);
}

Widget* WidgetPanel::next() {
	if(mi_focus!=mv_elem.end()) { // unfocus:
		Widget* pWidget = dynamic_cast<Widget*>(*mi_focus);
		if(pWidget) pWidget->leave();
	}
	// search next child widget:
	Widget* pWidget = 0;
	if(mv_elem.size()) for(vector<Element2D*>::iterator it = mi_focus+1; it!=mi_focus; ++it) {
		if(it>=mv_elem.end()) it=mv_elem.begin();
		pWidget = dynamic_cast<Widget*>(*it);
		if(!pWidget) continue;
		mi_focus = it;
		switchFocus(pWidget);
		break;
	}
	return pWidget;
}

Widget* WidgetPanel::previous() {
	if(mi_focus!=mv_elem.end()) { // unfocus:
		Widget* pWidget = dynamic_cast<Widget*>(*mi_focus);
		if(pWidget) pWidget->leave();
	}
	// search next child widget:
	Widget* pWidget = 0;
	if(mv_elem.size()) for(vector<Element2D*>::iterator it = (mi_focus == mv_elem.begin()) ? mv_elem.end()-1 : mi_focus-1; it!=mi_focus; --it) {
		if(it<mv_elem.begin()) it=mv_elem.end()-1;
		pWidget = dynamic_cast<Widget*>(*it);
		if(!pWidget) continue;
		mi_focus = it;
		switchFocus(pWidget);
		break;
	}
	return pWidget;
}


//--- class WidgetStack --------------------------------------------

const char* WidgetStack::s_type = "stack";

int WidgetStack::orientationEncode(string s) {
	toLower(s);
	return (s=="horizontal") ? 0 : (s=="vertical") ? 1 : -1;
}

std::string WidgetStack::orientationDecode(int code) {
	switch(code) {
		case 0 : return "horizontal";
		case 1: return "vertical";
		default: return string();
	}
}

void WidgetStack::init() {
	pair<float,float> dim = calcDim();
	width(dim.first);
	height(dim.second);
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
	float spacing = max(gui.style(m_style).measure(WidgetStyle::SPACING), 0.0f); 
	float padding = max(gui.style(m_style).measure(WidgetStyle::PADDING), 0.0f); 
	
	Element2D::init();
	//cout << "wSum:" << wSum << " hSum:" << hSum << " wMax:" << wMax << " hMax:" << hMax << endl;
	// calculate positions of children:
	if(m_vertical) {
		float posY=height()-padding;
		for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
			Element2D & elem = **it;
			elem.xRel(0.5f);
			elem.y(posY);
			elem.width(dim.first-2.0f*padding);
			elem.align(Element2D::ALIGN_CENTER, Element2D::ALIGN_TOP);
			elem.init();
			float h = elem.height();
			ElementText* pText=dynamic_cast<ElementText*>(*it); // FIXME ugly workaround for texts
			if(pText) h = pText->font().height(pText->text().c_str(), pText->width(), pText->lineHeight());
			posY-=h+spacing;
		}
	}
	else {
		float posX=padding;
		for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
			Element2D & elem = **it;
			elem.x(posX);
			elem.yRel(0.5f);
			elem.height(dim.second-2.0f*padding);
			elem.align(Element2D::ALIGN_LEFT, Element2D::ALIGN_CENTER);
			elem.init();
			posX+=elem.width()+spacing;
		}
	}
	WidgetPanel::init();
	// const Color & c = mp_background->color(); cout << c.r << " " << c.g << " " << c.b << " " << c.a << endl;
}

std::pair<float, float> WidgetStack::calcDim() const {
	// collect dimensions of child elements:
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
	float spacing = max(gui.style(m_style).measure(WidgetStyle::SPACING), 0.0f); 
	float padding = max(gui.style(m_style).measure(WidgetStyle::PADDING), 0.0f); 
	float wSum=2.0f*padding, hSum=2.0f*padding;
	float wMax=0.0f, hMax=0.0f;
		
	for(vector<Element2D*>::const_iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
		float w = (*it)->width();
		float h = (*it)->height();
		ElementText* pText=dynamic_cast<ElementText*>(*it);
		if(pText&&pText->width()) // FIXME ugly workaround for texts
			h = pText->font().height(pText->text().c_str(), pText->width(), pText->lineHeight());
		if(w>wMax) wMax = w;
		if(h>hMax) hMax = h;
		//cout << (*it)->name() << " w:" << w << " h:" << h << endl;
		wSum+=w;
		hSum+=h;
		if(it!=mv_elem.begin()) {
			wSum+=spacing;
			hSum+=spacing;
		}
	}
	if(m_vertical) return make_pair(wMax+2.0f*padding, hSum);
	else return make_pair(wSum, hMax+2.0f*padding);
}

//--- class WidgetPopup --------------------------------------------

const char* WidgetPopup::s_type = "popup";

int WidgetPopup::update(float pointerX, float pointerY, int buttonDown) { 
	if(m_state==STATE_DISABLED) return m_state;
	int state = STATE_NORMAL;
	if((pointerX>=m_bounding[0])&&(pointerY>=m_bounding[1])&&(pointerX<m_bounding[2])&&(pointerY<m_bounding[3]))
		state = STATE_FOCUS;
	else if(m_open&&(state<=STATE_FOCUS)) {
		toLocal(pointerX, pointerY);
		state = m_stack.update(pointerX,pointerY, buttonDown);
		if(state>Widget::STATE_NORMAL) m_value = m_stack.value();
	}

	if(buttonDown && (state>Widget::STATE_NORMAL)) {
		if(!m_open) open();
		else close();
	}
	if(state!=m_state) this->state(state);
	return m_state; 
}

void WidgetPopup::state(int st) { 
	if(st==m_state) return; 
	if((st<=Widget::STATE_NORMAL)&&(m_state>Widget::STATE_NORMAL)) {
		close();
		m_value.clear();
	}
	WidgetButton::state(st);
}

bool WidgetPopup::open() {
	//cout << "  WidgetPopup::open() " << m_name << ", open:" << m_open << endl;
	if(m_open) return false;
	mv_elem.push_back(&m_stack);
	m_stack.parent(this);
	if(m_stack.orientation()==WidgetStack::HORIZONTAL) {
		m_stack.align(Element2D::ALIGN_RIGHT, Element2D::ALIGN_CENTER);
		m_stack.xRel(0.0f);
		m_stack.yRel(0.5f);
	}
	else {
		int alignX=Element2D::ALIGN_CENTER;
		m_stack.xRel(0.5f);
		if(yAbs()+height()*0.5f<m_root.height()*0.5f) {
			m_stack.align(alignX, Element2D::ALIGN_BOTTOM);
			m_stack.yRel(1.0f);
		}
		else {
			m_stack.align(alignX, Element2D::ALIGN_TOP);
			m_stack.yRel(0.0f);
		}
	}
	m_stack.state(STATE_NORMAL);
	m_stack.init();
	// correct position if popup is outside of screen:
	float wHalf = m_stack.width()*0.5f;
	if(m_stack.xAbs()-wHalf < 0.0f) {
		float delta = -m_stack.xAbs()+wHalf;
		m_stack.x(m_stack.x()+delta);
		m_stack.init();
	}
	else if(m_stack.xAbs()+wHalf > m_root.width()) {
		float delta = m_stack.xAbs()+wHalf - m_root.width();
		m_stack.x(m_stack.x()-delta);
		m_stack.init();
	}
	m_stack.enter();
	m_open=true;
	return true;
}

bool WidgetPopup::close() {
	//cout << "  WidgetPopup::close() " << m_name << ", open:" << m_open << endl;
	if(!m_open) return false;
	mv_elem.pop_back();
	m_stack.parent(0);
	for(unsigned int i=0; i<m_stack.size(); ++i) {
		Widget* pWidget = dynamic_cast<Widget*>(m_stack[i]);
		if(!pWidget) continue;
		pWidget->state(STATE_NORMAL);
	}
	m_stack.state(STATE_DISABLED);
	m_open=false;
	return true;
}

Widget* WidgetPopup::enter() { 
	if(m_open) {
		m_value = m_stack.value();
		Widget* pWidget = m_stack.enter();
		return (pWidget==&m_stack) ? this : pWidget;
	}
	open();
	return this;
}

//--- class WidgetList ---------------------------------------------

const char* WidgetList::s_type = "list";

void WidgetList::init() {
	WidgetStack::init();
	if(!m_vertical||!m_pageSize||(mv_entry.size()<m_pageSize)) return;
	// calculate maximum width for all pages:
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
	const WidgetStyle & style = gui.style(m_contentStyle.size() ? m_contentStyle : m_style);
	float padding = max(style.measure(WidgetStyle::PADDING), 0.0f);
	float wMax=0.0f;
	for(unsigned int i=0; i<mv_entry.size(); ++i) {
		float w = style.font().width(mv_entry[i].first);
		if(w>wMax) wMax=w;
	}
	wMax+=2*padding;
	width(wMax);
	for(vector<Element2D*>::iterator it=mv_elem.begin(); it!=mv_elem.end(); ++it) {
		Element2D & elem = **it;
		elem.width(wMax);
	}
}

void WidgetList::appendString(const std::string & label, const std::string & value) {
	mv_entry.push_back(make_pair(label, value));
}

void WidgetList::dir(const std::string & path, const std::string & filter) {
	string originalDir;
	if(path.size()) {
		originalDir=io::cwd();
		io::chdir(path);
	}
	m_first=0;
	mv_entry.clear();
    
	string selDir=io::cwd();    

	//vector<string> vDir = io::dir(selDir);
	//for(unsigned int i=0; i<vDir.size(); ++i) if(io::isDir(selDir+'/'+vDir[i])) 
	//	appendString('[' + vDir[i] + ']', ".dir( \""+cmd+"\", \""+selDir+'/'+vDir[i]+"\", \""+filter+"\" )");
	
	// parse entries considering filter:
	vector<string> vDir = io::dir(selDir, filter);
	for(unsigned int i=0; i<vDir.size(); ++i)
		if(!io::isDir(selDir+'/'+vDir[i])) appendString( vDir[i], replaceAll(m_value,"$1$","\""+selDir+'/'+vDir[i]+"\"" ));
			
	if(path.size()) io::chdir(originalDir);
	pageUpdate();
	if(mp_parent) mp_parent->init();
}

bool WidgetList::load(const Var & v) {
	if((v.type()!=Var::ARRAY)||!v.size()) return false;
	mv_entry.clear();
	for(unsigned int i=0; i<v.size(); ++i) appendString( v[i].string(), replaceAll(m_value,"$1$", v[i].string() ));
	pageUpdate();
	if(mp_parent) mp_parent->init();
	return true;
}

bool WidgetList::pageNext() {
	if(!m_pageSize||(mv_entry.size()<m_pageSize)) return false;
	if(m_first+m_pageSize<mv_entry.size()) m_first+=m_pageSize;
	else m_first=0;
	pageUpdate();
	return true;
}

bool WidgetList::pagePrev() {
	if(!m_pageSize||(mv_entry.size()<m_pageSize)) return false;
	if(m_first>=m_pageSize) m_first-=m_pageSize;
	else if(m_first) m_first=0;
	else m_first = mv_entry.size()-m_pageSize;
	pageUpdate();
	return true;
}

void WidgetList::pageUpdate() {
	float w=width();
	clear();
	unsigned int last = m_pageSize ? min(mv_entry.size(), m_first+m_pageSize) : mv_entry.size();
	for(unsigned int i=m_first; i<last; ++i) {
		WidgetButton* pButton = new WidgetButton(m_root);
		pButton->style(m_contentStyle.size() ? m_contentStyle : m_style);
		pButton->text(mv_entry[i].first);
		pButton->value(mv_entry[i].second);
		append(pButton);
		if(w) pButton->width(w);
	}
	init();
}

//--- class WidgetInput --------------------------------------------

const char* WidgetInput::s_type = "input";

void WidgetInput::init() {
	m_tNow=0.0f;
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
	const WidgetStyle & style = gui.style(m_style);
	color(style.color(WidgetStyle::COLOR_BACKGR, m_state));
	if(!mp_label) {
		float padding =style.measure(WidgetStyle::PADDING);
		mp_label = new ElementString(gui, style.font(), "label");
		mp_label->x(padding);
		mp_label->yRel(0.5f);
		mp_label->width(width()-2*padding);
		mp_label->heightRel(1.0f);
		mp_label->align(Element2D::ALIGN_LEFT, Element2D::ALIGN_CENTER);
		mp_label->text(m_text);
		mp_label->color(style.color(WidgetStyle::COLOR_FOREGR, m_state)); 
		append(mp_label);
	}
	Widget::init();
}

void WidgetInput::state(int st) { 
	if(st==m_state) return; 
	const CanvasGui& gui = dynamic_cast<const CanvasGui&>(m_root);
	const WidgetStyle & style = gui.style(m_style);
	if(mp_label) {
		mp_label->color(style.color(WidgetStyle::COLOR_FOREGR, st)); 
		mp_label->text(m_text);
	}
	if(mp_background) {
		ElementPanel * pPanel = dynamic_cast<ElementPanel*>(mp_background);
		if(pPanel) Panel_applyStyle(*pPanel, m_style, st);
		else mp_background->color(style.color(WidgetStyle::COLOR_BACKGR, st));
	}
	else color(style.color(WidgetStyle::COLOR_BACKGR, st));
	Widget::state(st);
}

void WidgetInput::update(double deltaT) {
	Widget::update(deltaT);
	if((m_state<Widget::STATE_FOCUS)||!mp_label) return;
	if(static_cast<int>(m_tNow)%2 != static_cast<int>(m_tNow+deltaT)%2) {
		mp_label->text(m_text+( (static_cast<int>(m_tNow)%2) ? "_" : ""));
	}
	m_tNow+=deltaT;
}

string WidgetInput::value() const {
	return m_value.size() ? replaceAll(m_value,"$1$", m_text ) : m_text;
}
