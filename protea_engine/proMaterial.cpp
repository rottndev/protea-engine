#include "proStr.h"
#include "proResource.h"
#include "proMaterial.h"

using namespace std;

//--- class proMatData ----------------------------------------------

proMatData proMatData::s_default("default");

proMatData::proMatData(const std::string & name) : 
    m_name(name), 
    m_color(0.8f,0.8f,0.8f,1.0f),
    m_ambient(-0.2f,0.2f,0.2f),
    m_specular(1.0f,1.0f,1.0f),
    m_shininess(0.0f),
    m_emissive(0.0f,0.0f,0.0f),
    m_texId(0), m_texScale(1.0f,1.0f),
    m_texTransparent(false), m_refCount(1) { 
    m_texRepeat[0]=m_texRepeat[1]=true;
}

Xml proMatData::xml() const {
    Xml xApp("Appearance");
    xApp.attr("DEF",m_name);
    
    Xml xs("Material");
    xs.attr("diffuseColor",f2s(m_color[0])+' '+ f2s(m_color[1]) + ' ' + f2s(m_color[2]));
    if(m_color[3]<1.0f)
        xs.attr("transparency",f2s(1.0f-m_color[3]));
    if(m_ambient[0]>0.0f)
        xs.attr("ambientColor",f2s(m_ambient[0])+' '+f2s(m_ambient[1])+' '+f2s(m_ambient[2]));
    if((m_specular[0]!=1.0f)||(m_specular[1]!=1.0f)||(m_specular[2]!=1.0f))
        xs.attr("specularColor",f2s(m_specular[0])+' '+f2s(m_specular[1])+' '+f2s(m_specular[2]));
    if(m_shininess)
        xs.attr("shininess",f2s(m_shininess));
    if(m_emissive[0]||m_emissive[1]||m_emissive[2])
        xs.attr("emissiveColor",f2s(m_emissive[0])+' '+f2s(m_emissive[1])+' '+f2s(m_emissive[2]));
    xApp.append(xs);

    if(m_url.size()) {
        Xml xTex("ImageTexture");
        xTex.attr("url",m_url);
        xTex.attr("texScale",f2s(m_texScale[0])+' '+f2s(m_texScale[1]));
        xTex.attr("repeatS",m_texRepeat[0]?"TRUE":"FALSE");
        xTex.attr("repeatT",m_texRepeat[1]?"TRUE":"FALSE");
        xApp.append(xTex);
    }
    return xApp;
}

std::string proMatData::vrml(unsigned int nTabs) const {
    unsigned int i;
    string tabs;
    for(i=0; i<nTabs; i++) tabs+='\t';

    string s(tabs);
    if(m_name.size()) s+="DEF "+m_name+" ";

    s+=tabs+"Appearance {\n";
    s+=tabs+"\tmaterial Material {\n";
    s+=tabs+"\t\tdiffuseColor " + f2s(m_color[0])+' '+ f2s(m_color[1]) + ' ' + f2s(m_color[2]) + '\n';
    if(m_color[3]<1.0f)
        s+=tabs+"\t\ttransparency " + f2s(1.0-m_color[3]) + '\n';
    if((m_ambient[0]!=0.2f)||(m_ambient[1]!=0.2f)||(m_ambient[2]!=0.2f))
        s+=tabs+"\t\tambientColor "+f2s(m_ambient[0])+' '+ f2s(m_ambient[1]) + ' ' + f2s(m_ambient[2]) +'\n';
    if((m_specular[0]!=1.0f)||(m_specular[1]!=1.0f)||(m_specular[2]!=1.0f))
        s+=tabs+"\t\tspecularColor " + f2s(m_specular[0])+' '+ f2s(m_specular[1]) + ' ' + f2s(m_specular[2]) + '\n';
    if(m_shininess)
        s+=tabs+"\t\tshininess " + f2s(m_shininess) + '\n';
    if(m_emissive[0]||m_emissive[1]||m_emissive[2])
        s+=tabs+"\t\temissiveColor " + f2s(m_emissive[0])+' '+ f2s(m_emissive[1]) + ' ' + f2s(m_emissive[2]) + '\n';
    s+=tabs+"\t}\n";
    if(m_url.size()) {
        s+=tabs+"\ttexture ImageTexture {\n";
        s+=tabs+"\t\turl [ \""+m_url+"\" ]\n";
        s+=tabs+"\t\trepeatS "+string(m_texRepeat[0]?"TRUE":"FALSE")+" repeatT "+string(m_texRepeat[1]?"TRUE":"FALSE")+"\n";
        s+=tabs+"\t}\n";
    }
    s+=tabs+"}\n";
    return s;
}

bool proMatData::operator==(const proMatData &mat) const {
    if(mat.m_color!=m_color) return false;
    if(mat.m_ambient!=m_ambient) return false;
    if(mat.m_specular!=m_specular) return false;
    if(mat.m_shininess!=m_shininess) return false;
    if(mat.m_emissive!=m_emissive) return false;
    if(mat.m_url!=m_url) return false;
    if(mat.m_texId!=m_texId) return false;
    if(mat.m_texScale!=m_texScale) return false;
    if(mat.m_texRepeat[0]!=m_texRepeat[0]) return false;
    if(mat.m_texRepeat[1]!=m_texRepeat[1]) return false;
    return true;
}


//--- class proMaterial ---------------------------------------------

proMaterial::proMaterial(const Xml & xs) {
    m_data=&proMatData::s_default;
    set(xs);
}
    
proMaterial::proMaterial(const proMaterial & source) {
    m_data=source.m_data;
    if(m_data!=&proMatData::s_default)
        ++(m_data->m_refCount);
}

proMaterial::~proMaterial() {
    if(m_data!=&proMatData::s_default) {
        --(m_data->m_refCount);
        if(!m_data->m_refCount)
            delete m_data;
    }
}

const proMaterial & proMaterial::operator=(const proMaterial & source) {
    if(m_data!=&proMatData::s_default) {
        --(m_data->m_refCount);
        if(!m_data->m_refCount)
            delete m_data;
    }
    m_data=source.m_data;
    if(m_data!=&proMatData::s_default)
        ++(m_data->m_refCount);
    return *this;
}

void proMaterial::set(const proMaterial & source) {
    if(m_data!=&proMatData::s_default) {
        --(m_data->m_refCount);
        if(!m_data->m_refCount)
            delete m_data;
    }
    m_data=new proMatData(*source.m_data);
    m_data->m_refCount=1;
}

void proMaterial::set(const Xml & xs) {   
    const Xml & xTex=xs.child("ImageTexture") ? *xs.child("ImageTexture") : xs;
    const Xml & xMat=xs.child("Material") ? *xs.child("Material") : xs;

    string matName=xs.attr("DEF").size() ? xs.attr("DEF") 
        : xs.attr("id").size() ? xs.attr("id")
        : xMat.attr("DEF").size() ? xMat.attr("DEF") 
        : xMat.attr("id");
    if(m_data==&proMatData::s_default) m_data=new proMatData(matName);
    else m_data->m_name=matName;

    vector<string> vStr;
    if(xMat.attr("diffuseColor").size()) {
        vStr.clear();
        split(xMat.attr("diffuseColor"),vStr,", \t\n\015");
        if(vStr.size()>2)
            m_data->m_color.set(s2f(vStr[0]),s2f(vStr[1]),s2f(vStr[2]),1.0f);
    }
    else m_data->m_color.set(0.8f,0.8f,0.8f,1.0f);
    m_data->m_color[3]=1.0f-s2f(xMat.attr("transparency"));
    if(xMat.attr("ambientColor").size()) {
        vStr.clear();
        split(xMat.attr("ambientColor"),vStr,", \t\n\015");
        if(vStr.size()>2)
            m_data->m_ambient.set(s2f(vStr[0]),s2f(vStr[1]),s2f(vStr[2]),1.0f);
    }
    if(xMat.attr("specularColor").size()) {
        vStr.clear();
        split(xMat.attr("specularColor"),vStr,", \t\n\015");
        if(vStr.size()>2)
            m_data->m_specular.set(s2f(vStr[0]),s2f(vStr[1]),s2f(vStr[2]));
    }
    else m_data->m_specular.set(1.0f,1.0f,1.0f);
    if(xMat.attr("shininess").size())
        m_data->m_shininess=s2f(xMat.attr("shininess"));
    else m_data->m_shininess=0.0f;
    
    if(xMat.attr("emissiveColor").size()) {
        vStr.clear();
        split(xMat.attr("emissiveColor"),vStr,", \t\n\015");
        if(vStr.size()>2)
            m_data->m_emissive.set(s2f(vStr[0]),s2f(vStr[1]),s2f(vStr[2]));
    }
    else m_data->m_emissive.set(0.0f,0.0f,0.0f);
    
    m_data->m_url=xTex.attr("url");
    if(xTex.attr("texScale").size()) {
        vStr.clear();
        split(xTex.attr("texScale"),vStr,", \t\n\015");
        if(vStr.size()>1) m_data->m_texScale.set(s2f(vStr[0]),s2f(vStr[1]));
    }
    else m_data->m_texScale.set(1.0f,1.0f);
    if(xTex.attr("repeatS").size())
        m_data->m_texRepeat[0]=s2b(xTex.attr("repeatS"));
    else m_data->m_texRepeat[0]=true;
    if(xTex.attr("repeatT").size())
        m_data->m_texRepeat[1]=s2b(xTex.attr("repeatT"));
    else m_data->m_texRepeat[1]=true;
}

unsigned int proMaterial::loadTexture(bool reload) {
	if(!m_data->m_url.size()||(!reload&&m_data->m_texId)) return m_data->m_texId;
	// upload texture and get m_texId:
	m_data->m_texId = TextureMgr::singleton().getTextureId(m_data->m_url, m_data->m_texRepeat[0], m_data->m_texRepeat[1], reload);
	if(!m_data->m_texId)
		cerr << "WARNING proMaterial::loadTexture(): failed to open texture \"" << m_data->m_url << "\"\n";
	else {
		unsigned int id,w,h,d;
		TextureMgr::singleton().properties(m_data->m_url, id,w,h,d);
		if((d==1)||(d==2)||(d==4)) m_data->m_texTransparent = true;
	}
	return m_data->m_texId;
}

//--- class MaterialMgr ----------------------------------------

MaterialMgr* MaterialMgr::sp_instance=0;

void MaterialMgr::interpret(const Xml & xs) {
    for(unsigned int i=0; i<xs.nChildren(); ++i) 
        if(xs.child(i).first)
            if((xs.child(i).first->tag()=="Appearance")||(xs.child(i).first->tag()=="Material")) {
				bool found = false;
				for(unsigned int j=0; j<m_vMat.size(); ++j) {
					string matName=xs.child(i).first->attr("DEF");
					if(m_vMat[j].name()==matName) {
						m_vMat[j].set(*xs.child(i).first);
						found=true;
						break;
					}
				}
                if(!found) m_vMat.push_back(*xs.child(i).first);
			}
}

Xml MaterialMgr::xml() const {
    Xml xs("Materials");
    for(unsigned int i=0; i<m_vMat.size(); ++i)
        xs.append(m_vMat[i].xml());
    return xs;
}

size_t MaterialMgr::set(const proMaterial & mat) { 
    for(unsigned int i=0; i<m_vMat.size(); ++i)
        if(m_vMat[i].name()==mat.name()) {
            m_vMat[i]=mat;
            return i;
        }
    m_vMat.push_back(mat); 
    return m_vMat.size()-1;
}
 
size_t MaterialMgr::add(const proMaterial & mat) { 
    if(!mat.name().size()) return addAnonymous(mat);
    for(unsigned int i=0; i<m_vMat.size(); ++i)
        if(m_vMat[i].name()==mat.name()) return i;
    m_vMat.push_back(mat); 
    return m_vMat.size()-1;
}
 
size_t MaterialMgr::addAnonymous(const proMaterial & mat) { 
    for(unsigned int i=0; i<m_vMat.size(); ++i)
        if(m_vMat[i]==mat) return i;
    m_vMat.push_back(mat); 
    m_vMat.back().name("mat_"+i2s(m_counter++));
    return m_vMat.size()-1;
}
 
proMaterial & MaterialMgr::operator[](const string & s) {
    for(unsigned int i=0; i<m_vMat.size(); ++i)
        if(m_vMat[i].name()==s) return m_vMat[i];
    return m_vMat[0];
}

const proMaterial & MaterialMgr::operator[](const string & s) const {
    for(unsigned int i=0; i<m_vMat.size(); ++i)
        if(m_vMat[i].name()==s) return m_vMat[i];
    return m_vMat[0];
}

unsigned int MaterialMgr::getId(const string & s) const {
    for(unsigned int i=0; i<m_vMat.size(); ++i)
        if(m_vMat[i].name()==s) return i;
    return 0;
}
