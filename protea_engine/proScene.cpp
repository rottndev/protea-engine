#include "proScene.h"
#include "proStr.h"
#include "proMesh.h"
#include "proRenderer.h"
#include <map>
#include <climits>
using namespace std;

//--- class proCamera -----------------------------------------------

const char* const proCamera::TYPE = "camera";

proCamera::proCamera() : proNode(), m_tNow(0.0), m_currMat(0), m_flags(~(FLAG_SHADOW|FLAG_WIREFRAME)), m_pLight(0) { 
	if(proNode::renderer()) 
		mp_renderable = proNode::renderer()->create(*this);
}

void proCamera::push(const mat4f & matrix) {
    if(m_currMat+1<s_nMat) {
        m_mat[m_currMat+1]=m_mat[m_currMat]*matrix;
        ++m_currMat;
		if(mp_renderable)
			mp_renderable->push(matrix);
    }
}

void proCamera::pop() { 
    if(m_currMat) {
        --m_currMat; 
		if(mp_renderable)
			mp_renderable->pop();
    }
}

void proCamera::update() {
	if(mp_renderable)
		mp_renderable->update();
    m_frustum.set(m_dim[0]*m_dim[4], m_dim[1]*m_dim[4], m_dim[2]*m_dim[4], m_dim[3]*m_dim[4], m_dim[4], m_dim[5]);
    m_frustum.rotate(0,90,0);
    m_frustum.transform(m_pos);
}
 
line proCamera::ray(float relX, float relY) const {
    // translate according to frustum:
    relX=(m_dim[0]+((relX+1.0f)*0.5f)*(m_dim[1]-m_dim[0]))*m_dim[4];
    relY=(m_dim[2]+((relY+1.0f)*0.5f)*(m_dim[3]-m_dim[2]))*m_dim[4];
    return line(m_pos,m_pos+ direction()*m_dim[4] +(right()*relX)+(up()*relY));
}

vec3f proCamera::direction() const {
	vec3f v(0.0f, 1.0f, 0.0f);
	v.rotate(m_pos[R], vec3f(0.0f,1.0f,0.0f));
	v.rotate(m_pos[P], vec3f(1.0f,0.0f,0.0f));
	v.rotate(m_pos[H], vec3f(0.0f,0.0f,1.0f));
	return v;
}

vec3f proCamera::right() const {
	vec3f v(1.0f, 0.0f, 0.0f);
	v.rotate(m_pos[R], vec3f(0.0f,1.0f,0.0f));
	v.rotate(m_pos[P], vec3f(1.0f,0.0f,0.0f));
	v.rotate(m_pos[H], vec3f(0.0f,0.0f,1.0f));
	return v;
}

vec3f proCamera::up() const {
	vec3f v(0.0f, 0.0f, 1.0f);
	v.rotate(m_pos[R], vec3f(0.0f,1.0f,0.0f));
	v.rotate(m_pos[P], vec3f(1.0f,0.0f,0.0f));
	v.rotate(m_pos[H], vec3f(0.0f,0.0f,1.0f));
	return v;
}


//--- class proLight ------------------------------------------------

const char* const proLight::TYPE = "light";

proLight::proLight(const vec4f & position) : 
	proNode(),
    m_pos(position), 
    m_amb(0.3f,0.3f,0.3f,1.0f), 
    m_dif(0.7f,0.7f,0.7f,1.0f),
    m_spc(1.0,1.0,1.0,1.0),
    m_shd(0.0f, 0.0f, 0.0f, 0.2f) { 
	m_flags=FLAG_ACTIVE|FLAG_UPDATE|FLAG_SHADOW|FLAG_LIGHT|FLAG_RENDER; 
}
        
proLight::proLight(const Xml & xs) : 
	proNode(),
    m_amb(0.0f,0.0f,0.0f,1.0f), 
    m_dif(0.7f,0.7f,0.7f,1.0f),
    m_spc(1.0,1.0,1.0,1.0),
    m_shd(0.0f, 0.0f, 0.0f, 0.2f) { 
		
    m_name=xs.attr("DEF");
    m_flags=FLAG_ACTIVE|FLAG_UPDATE|FLAG_SHADOW|FLAG_LIGHT|FLAG_RENDER;
    bool isPointLight=(xs.tag()=="PointLight");
    vector<string> vStr;
    split(isPointLight ? xs.attr("location") : xs.attr("direction"),vStr);
    if(vStr.size()>2)
        m_pos.set(s2f(vStr[0]),-s2f(vStr[Z]),s2f(vStr[Y]), isPointLight ? 1.0f:0.0f);
    if(!isPointLight) m_pos*=-1.0f;
    if(xs.attr("color").size()) {
        vStr.clear();
        split(xs.attr("color"),vStr);
        float intensity=xs.attr("intensity").size() ? s2f(xs.attr("intensity")) : 1.0f;
        float ambIntens=xs.attr("ambientIntensity").size() ? s2f(xs.attr("ambientIntensity")) : 0.0f;
        m_dif.set(s2f(vStr[0])*intensity,s2f(vStr[1])*intensity,s2f(vStr[2])*intensity,1.0f);
        m_amb.set(s2f(vStr[0])*ambIntens,s2f(vStr[1])*ambIntens,s2f(vStr[2])*ambIntens,1.0f);
    }
    if(xs.attr("radius").size())    
        m_bndSphere.radius(s2f(xs.attr("radius")));
}
        
Xml proLight::xml() const {
    bool isPointLight=(m_pos[3]!=0.0f);
    Xml xs(isPointLight?"PointLight":"DirectionalLight");
    if(m_name.size()) xs.attr("DEF",m_name);
    if(isPointLight) xs.attr("location",f2s(m_pos[X]/m_pos[3])+' '+f2s(m_pos[Z]/m_pos[3])+' '+f2s(-m_pos[Y]/m_pos[3]));
    else xs.attr("direction",f2s(-m_pos[X])+' '+f2s(-m_pos[Z])+' '+f2s(m_pos[Y]));
    if(m_flags&FLAG_SHADOW) xs.attr("castShadow","TRUE");
    return xs;
}

void proLight::update() {
    m_bndSphere.center()=m_pos;
	proNode::update();
    if(!(m_flags&FLAG_SHADOW)) m_flags&=~FLAG_UPDATE;
}

//--- class proNode -------------------------------------------------

const char* const proNode::TYPE = "node";
Renderer * proNode::sp_renderer = 0;

Xml proNode::xml() const {
    Xml node("Node");
    if(m_name.size()) node.attr("DEF",m_name);
    return node;
}

void proNode::initGraphics() {
	if(mp_renderable) {
		delete mp_renderable;
		mp_renderable = 0;
	}
	if(sp_renderer) mp_renderable = sp_renderer->create(*this);
	calcBounding(false);
}

void proNode::draw(proCamera & camera) {
    if(!(m_flags&FLAG_ACTIVE)||!(m_flags&FLAG_RENDER)) return;
	if(mp_renderable) mp_renderable->draw(camera);
}

void proNode::update()  { 
	if(mp_renderable) mp_renderable->update();
}

void proNode::closeGraphics()  { 
	if(mp_renderable) { 
		delete mp_renderable; 
		mp_renderable=0; 
	} 
}

bool proNode::testBounding(const frustum & frust, const mat4f & matrix) const {
    if(m_bndSphere.radius()<0.0f) return true;
    sphere bounding(boundingSphere());
    bounding.transform(matrix);
    return frust.intersects(bounding);
}

/// auxiliary function that substitutes USE attributes by corresponding DEF attributes
static void substituteUse(Xml & xs, Xml & root) {
    unsigned int i;
    for(i=0; i<xs.nAttr(); ++i)
        if(xs.attr(i).first=="USE") {
            Xml *pDef=root.find("","DEF",xs.attr(i).second);
            if(pDef) {
                for(size_t j=0; j<pDef->nAttr(); ++j)
                    xs.attr(pDef->attr(j).first,pDef->attr(j).second);
                for(size_t j=0; j<pDef->nChildren(); ++j)
                    if(pDef->child(j).first) xs.append(*pDef->child(j).first);
                    else xs.append(pDef->child(j).second);
            }
            else cerr << "ioX3d::substituteUse() WARNING: no subTag DEF=" << xs.attr(i).first << " found.\n";
            xs.attr("USE","");
            break;
        }
    for(i=0; i<xs.nChildren(); ++i) if(xs.child(i).first) substituteUse(*xs.child(i).first,root);
}

proNode * proNode::interpret(const Xml & xs) {
	if((xs.tag()=="X3D")&&xs.find("Scene"))
		return interpret(*xs.find("Scene"));
	if(xs.tag()=="Scene") {
		Xml xSubst(xs);
		substituteUse(xSubst,xSubst);
		return new proTransform(xSubst);
	}
	if((xs.tag() == "Group")||(xs.tag() == "Transform"))
		return new proTransform(xs);
	if(xs.tag()=="Shape") {
		const Xml* xIndFaceSet=xs.find("IndexedFaceSet");
		if(xIndFaceSet) return interpret(*xIndFaceSet);
	}
	if(xs.tag()=="IndexedFaceSet")
		return new proMesh(xs);
	if((xs.tag()=="DirectionalLight")||(xs.tag()=="PointLight"))
		return new proLight(xs);
	return 0;
}


//--- class proTransform --------------------------------------------

const char* const proTransform::TYPE = "transform";

proTransform::proTransform(const proTransform & source) : proNode(source), m_mat(source.m_mat), m_isIdentity(source.m_isIdentity) {
    for(size_t i=0; i<source.mv_node.size(); ++i)
        mv_node.push_back(source.mv_node[i]->copy());
}

proTransform::proTransform(const Xml & xGet) : proNode(), m_isIdentity(true) {
    Xml xs(xGet);
    m_name=xs.attr("DEF");

    bool isScaled=false;
    vector<string> vStr;
    if(xs.attr("translation").size()) {
        //cerr << "translation " << xs.attr("translation") << "\n";
        vStr.clear();
        split(xs.attr("translation"),vStr);
        if(vStr.size()>2)
            m_mat.translate(s2f(vStr[0]),-s2f(vStr[Z]),s2f(vStr[Y]));
    }
    vec3f center;
    if(xs.attr("center").size()) {
        //cerr << "center " << xs.attr("center") << "\n";
        vStr.clear();
        split(xs.attr("center"),vStr);
        if(vStr.size()>2) {
            center.set(s2f(vStr[0]),-s2f(vStr[Z]),s2f(vStr[Y]));
            if(center[X]||center[Y]||center[Z])
                m_mat.translate(center);
        }
    }
    if(xs.attr("rotation").size()) {
        //cerr << "rotation " << xs.attr("rotation") << "\n";
        vStr.clear();
        split(xs.attr("rotation"),vStr);
        if(vStr.size()>3) {
            float angle=s2f(vStr[3])*RAD2DEG;
            if(angle)
                m_mat.rotate(angle, s2f(vStr[0]),-s2f(vStr[Z]),s2f(vStr[Y]));
        }
    }
    
    if(xs.attr("scale").size()) {
        vec4f scOri(0.0f,0.0f,0.0f,0.0f);
        if(xs.attr("scaleOrientation").size()) {
            //cerr << "scaleOrientation " << xs.attr("scaleOrientation") << "\n";
            vStr.clear();
            split(xs.attr("scaleOrientation"),vStr);
            if(vStr.size()>3) {
                scOri.set(s2f(vStr[3])*RAD2DEG, s2f(vStr[0]),-s2f(vStr[Z]),s2f(vStr[Y]));
                if(scOri[3]) m_mat.rotate(scOri[3],scOri[X],scOri[Y],scOri[Z]);
            }
        }
        //cerr << "scale " << xs.attr("scale") << "\n";
        vStr.clear();
        split(xs.attr("scale"),vStr);
        if(vStr.size()>2) {
            vec3f sc(s2f(vStr[X]),s2f(vStr[Z]),s2f(vStr[Y]));
            if(sc[X]||sc[Y]||sc[Z]) {
                isScaled=true;
                m_mat.scale(sc);
            }
        }
        if(scOri[3])
            m_mat.rotate(-scOri[3],scOri[X],scOri[Y],scOri[Z]);
    }
        
    if(center[X]||center[Y]||center[Z])
        m_mat.translate(-center);

    for(size_t i=0; i<xs.nChildren(); ++i) if(xs.child(i).first) {
        proNode* pChild=interpret(*xs.child(i).first);
        if(isScaled) pChild->transform(m_mat);
        append(pChild,false);
    }
    if(isScaled) m_mat.identity();
	else m_isIdentity=m_mat.isIdentity();
}

void proTransform::draw(proCamera & camera) {
    if(!(m_flags&FLAG_ACTIVE) || !mv_node.size()) return; 
    if((camera.flags()&FLAG_SHADOW) && camera.light()) { // draw shadow volumes
        proLight * pLightOrig=camera.light();
        proLight lightTr(*camera.light());
        if(!m_isIdentity) {
            camera.push(m_mat);
            mat4f matInv(m_mat);
            if(!camera.light()->pos()[3]) // do not consider translations for distant lights
                matInv[12]=matInv[13]=matInv[14]=0.0f;
            matInv.invert();
            if(!matInv.isNan()) {
                vec3f posTr(camera.light()->pos());
                posTr.transform(matInv);
                lightTr.pos(vec4f(posTr[X],posTr[Y],posTr[Z],camera.light()->pos()[3]));
                lightTr.flags()=camera.light()->flags();
                camera.light(&lightTr);
            }
        }
        for(vector<proNode*>::iterator it=mv_node.begin(); it!=mv_node.end(); ++it) 
            (*it)->draw(camera);
        if(!m_isIdentity) {
            camera.pop();
            camera.light(pLightOrig);
        }
        return;
    }
    
    // normal draw:
    if(!testBounding(camera.frs(),camera.matrix())) return;
    if(!m_isIdentity) camera.push(m_mat);
    for(vector<proNode*>::iterator it=mv_node.begin(); it!=mv_node.end(); ++it)
        (*it)->draw(camera);
    m_flags&=~FLAG_UPDATE;
    if(!m_isIdentity) camera.pop();
}

bool proTransform::erase(proNode * node, bool doDelete) {
    for(vector<proNode*>::iterator it=mv_node.begin(); it!=mv_node.end(); ++it)
        if(*it==node) {
            if(doDelete) delete *it;
            mv_node.erase(it);
            return true;
        }
    return false;
}

void proTransform::clear() {
    for(vector<proNode*>::iterator i=mv_node.begin(); i!=mv_node.end(); ++i)
        delete *i;
    mv_node.clear();
}

proNode * proTransform::next(proNode::iterator & iter) {
    if(!mv_node.size()) return 0;
    if(iter.topObj()==this) {
        iter.push(this);
        if(iter.topCounter()>=mv_node.size()) {
            iter.pop();
            return 0;
        }
        return mv_node[iter.topCounter()];
    }
    else {
        iter.push(this);
        return mv_node[0];
    }
}

void proTransform::enable(unsigned int flag) {
    m_flags|=flag;
    if((flag&FLAG_SHADOW)||(flag&FLAG_UPDATE))
        for(vector<proNode*>::iterator it=mv_node.begin(); it!=mv_node.end(); ++it)
            (*it)->enable(flag);
}

void proTransform::disable(unsigned int flag) {
    m_flags&=~flag;
    if(flag&FLAG_SHADOW)
        for(vector<proNode*>::iterator it=mv_node.begin(); it!=mv_node.end(); ++it)
            (*it)->disable(FLAG_SHADOW);
}

void proTransform::transform(const mat4f & mat) {
    for(vector<proNode*>::iterator it=mv_node.begin(); it!=mv_node.end(); ++it)
        (*it)->transform(mat);
    calcBounding(false);
}

void proTransform::calcBounding(bool recursive) {
    if(!mv_node.size()) {
        m_bndSphere.radius(-1.0f);
        m_bbox.first=m_bbox.second=vec3f(0.0f,0.0f,0.0f);
        return;
    }
    unsigned int i;
    m_bndSphere.radius(0.0f);
    float radiusMax=0.0f;
    if(recursive) mv_node[0]->calcBounding(true);
    vec3f pMin(mv_node[0]->boundingSphere().center());
    vec3f pMax(mv_node[0]->boundingSphere().center());
    m_bbox=mv_node[0]->boundingBox();
    for(i=0; i<mv_node.size(); ++i) {
        if((i>0) && recursive) mv_node[i]->calcBounding(true);
        if(mv_node[i]->boundingSphere().radius()<0.0f)
            m_bndSphere.radius(-1.0f);
        if(m_bndSphere.radius()>=0.0f) {
            // calculate bounding box parallel to coordinate system:
            m_bbox.first[X]=min(mv_node[i]->boundingBox().first[X],m_bbox.first[X]);
            m_bbox.second[X]=max(mv_node[i]->boundingBox().second[X],m_bbox.second[X]);
            m_bbox.first[Y]=min(mv_node[i]->boundingBox().first[Y],m_bbox.first[Y]);
            m_bbox.second[Y]=max(mv_node[i]->boundingBox().second[Y],m_bbox.second[Y]);
            m_bbox.first[Z]=min(mv_node[i]->boundingBox().first[Z],m_bbox.first[Z]);
            m_bbox.second[Z]=max(mv_node[i]->boundingBox().second[Z],m_bbox.second[Z]);

            pMin[X]=min(mv_node[i]->boundingSphere()[X],pMin[X]);
            pMax[X]=max(mv_node[i]->boundingSphere()[X],pMax[X]);
            pMin[Y]=min(mv_node[i]->boundingSphere()[Y],pMin[Y]);
            pMax[Y]=max(mv_node[i]->boundingSphere()[Y],pMax[Y]);
            pMin[Z]=min(mv_node[i]->boundingSphere()[Z],pMin[Z]);
            pMax[Z]=max(mv_node[i]->boundingSphere()[Z],pMax[Z]);
            radiusMax=max(radiusMax,mv_node[i]->boundingSphere().radius());
        }
    }
    if(m_bndSphere.radius()<0.0f) return;
    // search for minimal radius from center of bounding box:
    vec3f c((pMin+pMax)*0.5f);
    float rSquared=0.0f;
    for(i=0; i<mv_node.size(); ++i) {
        float currDist=c.sqrDistTo(mv_node[i]->boundingSphere().center());
        if(currDist>rSquared) rSquared=currDist;
    }
    m_bndSphere=sphere(c,sqrt(rSquared)+radiusMax);
    
    // apply scaling transform:
    vec3f vX(1,0,0), vY(0,1,0), vZ(0,0,1), vO(0,0,0);
    vX.transform(m_mat);
    vY.transform(m_mat);
    vZ.transform(m_mat);
    vO.transform(m_mat);
    vX-=vO; vY-=vO; vZ-=vO;
    float xl=vX.length(), yl=vY.length(), zl=vZ.length();
    float max3=xl>yl ? (xl>zl ? xl : zl) : (yl>zl ? yl : zl);
    m_bndSphere.radius(m_bndSphere.radius()*max3);
    m_bbox.first.transform(m_mat);
    m_bbox.second.transform(m_mat);
}

sphere proTransform::boundingSphere() const {
    if(m_isIdentity) return m_bndSphere;
    sphere bounding(m_bndSphere);
    bounding.transform(m_mat);
    return bounding;
}

bool proTransform::intersects(const line & ray) const {
    // first test on bounding level:
    if(!mv_node.size()||!ray.intersects(boundingSphere())) return false;
    line r(ray);
    if(!m_isIdentity) // transform ray into local coordinate system of selected node:
        r.transform(m_mat.inverse());
    // now test on individual subnodes:
    for(vector<proNode*>::const_iterator it=mv_node.begin(); it!=mv_node.end(); ++it)
        if((*it)->intersects(r)) return true;
    return false;
}

vec3f * proTransform::intersection(const line & ray) const {
    // first test on bounding level:
    if(!mv_node.size()||!ray.intersects(boundingSphere())) {
		return 0;
	}
    line r(ray);
    if(!m_isIdentity) // transform ray to local coordinate system of selected node:
        r.transform(m_mat.inverse());
    // now test on individual subnodes:
    float minSqrDist=FLT_MAX;
    vec3f * pMin=0;
    for(vector<proNode*>::const_iterator it=mv_node.begin(); it!=mv_node.end(); ++it) {
        vec3f * pCurr = (*it)->intersection(r);
        if(pCurr) {
            float currSqrDist=r[0].sqrDistTo(*pCurr);
            if( currSqrDist<minSqrDist ) {
                minSqrDist=currSqrDist;
                if(pMin) delete pMin;
                pMin = pCurr;
            }
            else delete pCurr;
        }
    }
    if(pMin) pMin->transform(m_mat);
    return pMin;
}

const proNode * proTransform::query(const line & ray, unsigned int queryFlags) const {
	//cout << "query on (" << name() << "): queryFlags:" << queryFlags << " m_queryFlags:" << m_queryFlags << " comb: " << (m_queryFlags&queryFlags) << endl;
	if(!(m_queryFlags&queryFlags))	return 0;
    // first test on bounding level:
    if(!mv_node.size()||((m_bndSphere.radius()>=0.0f)&&!ray.intersects(boundingSphere())))
		return 0;
	const proNode * pNearest = this;
    line r(ray);
    if(!m_isIdentity) {
		r.transform(m_mat.inverse()); // transform ray to local coordinate system
	}
    // now test on individual subnodes:
    float minSqrDist=FLT_MAX;
    for(vector<proNode*>::const_iterator it=mv_node.begin(); it!=mv_node.end(); ++it) {
        vec3f * pCurr = (*it)->intersection(r);
        if(pCurr) {
            float currSqrDist=r[0].sqrDistTo(*pCurr);
            if( currSqrDist<minSqrDist ) {
                minSqrDist=currSqrDist;
                if((*it)->queryFlags()&queryFlags) {
					pNearest = *it;
					if(pNearest->type()==proTransform::TYPE)
						pNearest = pNearest->query(r, queryFlags);
				}
            }
            delete pCurr;
        }
    }
    return pNearest;    
}

Xml proTransform::xml() const {
    Xml node(m_isIdentity ? "Group" : "Transform");
    if(m_name.size()) node.attr("DEF",m_name);
    Xml * pXml=&node;
    if(!m_isIdentity) {
        vec6f pos(m_mat);
        if(pos[X]||pos[Y]||pos[Z])
		node.attr("translation",f2s(pos[X])+' '+f2s(pos[Z])+' '+f2s(-pos[Y]));
        if(pos[H]) pXml->attr("rotation","0 1 0 "+f2s(DEG2RAD*pos[H]));
        if(pos[P]) {
		if(pXml->attr("rotation").size()) {
			Xml xChild("Transform");
			pXml->append(xChild);
			pXml=pXml->child(0).first;
		}
		pXml->attr("rotation","1 0 0 "+f2s(DEG2RAD*pos[P]));
        }
        if(pos[R]) {
		if(pXml->attr("rotation").size()) {
			Xml xChild("Transform");
			pXml->append(xChild);
			pXml=pXml->child(0).first;
		}
		pXml->attr("rotation","0 0 -1 "+f2s(DEG2RAD*pos[R]));
        }
    }
	
	for(size_t i=0; i<mv_node.size(); ++i)
		pXml->append(mv_node[i]->xml());
	return node;
}


//--- class proScene ------------------------------------------------

const char* const proScene::TYPE = "scene";


//--- class proMesh -----------------------------------------------

const char* const proMesh::TYPE = "mesh";

proMesh::proMesh(const std::string & name) : proNode(name), m_kind(KIND_INDEXED_TRIANGLES) { 
    m_flags|=FLAG_SHADOW|FLAG_ZFAIL|FLAG_RENDER|FLAG_COLLISION; 
}

proMesh::proMesh(const proMesh& source) : proNode(source), 
    m_kind(source.m_kind),
    mv_coord(source.mv_coord),
    mv_texCoord(source.mv_texCoord),
    mv_color(source.mv_color),
    mv_normal(source.mv_normal),
    mv_fNormal(source.mv_fNormal),
    mv_index(source.mv_index),
    mv_edge(source.mv_edge),
    mv_shadow(source.mv_shadow),
    mv_cap(source.mv_cap),
    m_mat(source.m_mat) { }

proMesh::proMesh(const Xml & xs) : proNode(), m_kind(KIND_INDEXED_TRIANGLES) {
    m_flags|=FLAG_SHADOW|FLAG_ZFAIL|FLAG_RENDER|FLAG_COLLISION;
    m_name=xs.attr("DEF");
    if(xs.tag()!="IndexedFaceSet")
        cerr << "proMesh constructor WARNING: statement has wrong tag: [" << xs.tag() << "]\n";

    // search necessary subtags:
    const Xml * coord=xs.find("Coordinate");
    bool abort=false;
    if(!coord) {
        cerr << "proMesh constructor ERROR: no Coordinate tag found.\n";
        abort=true;
    }
    if(!xs.attr("coordIndex").size()) {
        cerr << "proMesh constructor ERROR: no coordIndex tag found.\n";
        abort=true;
    }
    if(abort) return;

    // read into object:
	if(xs.attr("solid").size() && !s2b(xs.attr("solid"))) m_flags|= FLAG_FRONT_AND_BACK;
    vector<string> vStr;
    size_t i;
    split(coord->attr("point"),vStr,", \t\n\015");
    if(vStr.size()%3!=0)
        cerr << "proMesh constructor WARNING: number of coords not a multiple of 3.\n";
    else for(i=0; i<vStr.size(); i+=3)
        mv_coord.push_back(vec3f(s2f(vStr[i]),-s2f(vStr[i+2]),s2f(vStr[i+1]))); // flip YZ

    // add coordinate indices:
    vector<size_t> vFaceEnds;
    vStr.clear();
    split(xs.attr("coordIndex"),vStr,", \t\n\015");
    if(vStr.size()>0) {
        for(i=0; i<vStr.size()-1; ++i) {
            mv_index.push_back(s2ui(vStr[i]));
            if(vStr[i+1]=="-1") {
                vFaceEnds.push_back(mv_index.size()-1);
                ++i;
            }
        }
        if(i<vStr.size()) {
            int index=s2i(vStr[i]);
            if(index>=0) {
                mv_index.push_back(index);
                vFaceEnds.push_back(mv_index.size()-1);
            }
        }
    }
    
    // storage for further temporary indices:
    vector<size_t> vTexIndices;
    vector<size_t> vNormalIndices;
    vector<size_t> vColorIndices;
    
    const Xml * xsNormal=xs.parent()->find("Normal");
    if(xsNormal) {
        vector<float> vNormal;
        s2f(xsNormal->attr("vector"),vNormal);
        mv_normal.reserve(vNormal.size()/3);
        if(vNormal.size()%3!=0)
            cerr << "proMesh constructor WARNING: number of normals not a multiple of 3.\n";
        else for(i=0; i+2<vNormal.size(); i+=3)
            mv_normal.push_back(vec3f(vNormal[i],-vNormal[i+2],vNormal[i+1]));
        if(xs.attr("normalIndex").size()) {
            vStr.clear();
            split(xs.attr("normalIndex"),vStr,", \t\n\015");
            for(i=0; i<vStr.size(); i++) {
                int index=s2i(vStr[i]);
                if(index>=0) vNormalIndices.push_back(index);
            }
        }
    }

    // add color per vertex information:
    const Xml * colorValues=xs.find("Color");
    if((toUpper(xs.attr("colorPerVertex"))!="FALSE")&&colorValues) {
        vector<float> vColors;
        s2f(colorValues->attr("color"),vColors);
        mv_color.reserve(vColors.size()/3);
        for(i=0;i<vColors.size()-2;i+=3)
            mv_color.push_back(vec3f(vColors[i],vColors[i+1],vColors[i+2]));

        if(xs.attr("colorIndex").size()) {
            vStr.clear();
            split(xs.attr("colorIndex"),vStr,", \t\n\015");
            for(i=0; i<vStr.size(); i++) {
                int index=s2i(vStr[i]);
                if(index>=0) vColorIndices.push_back(index);
            }
        }
    }

    // parse texture coordinate information:
    const Xml * texCoord=xs.find("TextureCoordinate");
    if(texCoord) {
        vector<float> texCoords;
        s2f(texCoord->attr("point"),texCoords);
        mv_texCoord.reserve(texCoords.size()/2);
        for(i=0; i<texCoords.size(); i+=2) mv_texCoord.push_back(vec2f(texCoords[i],texCoords[i+1]));

        if(xs.attr("texCoordIndex").size()) {
            vStr.clear();
            split(xs.attr("texCoordIndex"),vStr,", \t\n\015");
            for(i=0; i<vStr.size(); i++) {
                int index=s2i(vStr[i]);
                if(index>=0) vTexIndices.push_back(index);
            }
        }
    }
    
    // add material information:
    if(xs.parent()) {
        const Xml * xMat=xs.parent()->find("Appearance");
        if(xMat) // allow shared materials:
            m_mat=MaterialMgr::singleton()[MaterialMgr::singleton().add(*xMat)];
    }

    // normalize between various indices:
	if(vTexIndices.size()&&(vTexIndices.size()!=mv_index.size()))
        vTexIndices.clear();
    if(vNormalIndices.size()&&(vNormalIndices.size()!=mv_index.size()))
        vNormalIndices.clear();
    if(vColorIndices.size()&&(vColorIndices.size()!=mv_index.size()))
        vColorIndices.clear();
    if(vTexIndices.size()||vNormalIndices.size()||vColorIndices.size()) {
        vector<vec3f> vCoord;
        vCoord.reserve(mv_index.size());
        vector<vec2f> vTexCoord;
        vTexCoord.reserve(mv_index.size());
        vector<vec3f> vNormal;
        vNormal.reserve(mv_index.size());
        vector<vec3f> vColor;
        vColor.reserve(mv_index.size());
		vector<unsigned int> vIndex;
		vIndex.reserve(mv_index.size());
        for(i=0; i<mv_index.size(); ++i) { // normalize indices:
            // search for suitable vertex:
            size_t index=UINT_MAX;
            for(size_t j=0; j<vCoord.size(); ++j) {
                if(vCoord[j]!=mv_coord[mv_index[i]]) 
                    continue;
                if(mv_texCoord.size()&&(vTexCoord[j]!=mv_texCoord[vTexIndices.size() ? vTexIndices[i] : mv_index[i]]))
                    continue;
                if(mv_normal.size()&&(vNormal[j]!=mv_normal[vNormalIndices.size() ? vNormalIndices[i] : mv_index[i]]))
                    continue;
                if(mv_color.size()&&(vColor[j]!=mv_color[vColorIndices.size() ? vColorIndices[i] : mv_index[i]]))
                    continue;
                index=j;
                break;
            }
            if(index==UINT_MAX) { // no suitable vertex found, add new:
                index=static_cast<unsigned int>(vCoord.size());
                vCoord.push_back(mv_coord[mv_index[i]]);
                if(mv_texCoord.size())
                    vTexCoord.push_back(mv_texCoord[vTexIndices.size() ? vTexIndices[i] : mv_index[i]]);
                if(mv_normal.size())
                    vNormal.push_back(mv_normal[vNormalIndices.size() ? vNormalIndices[i]:mv_index[i]]);
                if(mv_color.size())
                    vColor.push_back(mv_color[vColorIndices.size() ? vColorIndices[i] : mv_index[i]]);
            }
			vIndex.push_back(index);
		}
        mv_coord=vCoord;
        if(mv_texCoord.size()) mv_texCoord=vTexCoord;
        if(mv_normal.size()) mv_normal=vNormal;
        if(mv_color.size()) mv_color=vColor;
		mv_index=vIndex;
	}
    // triangulate vertex indices based on face ends:
	vector<unsigned int> vIndex;
    // rebuild indices list:
	size_t currStart=0;
	size_t currEnd=0;
	for(size_t i=0; (i<mv_index.size())&&(currEnd<vFaceEnds.size()); ++i) {
		if(i>currStart+1) {
			vIndex.push_back(mv_index[currStart]);
			vIndex.push_back(mv_index[i-1]);
			vIndex.push_back(mv_index[i]);
		}
		if(i==vFaceEnds[currEnd]) {
			++currEnd;
			currStart=i+1;
		}
	}
	mv_index=vIndex;
}

void proMesh::initGraphics() {
	proNode::initGraphics();

	if(mv_fNormal.size()*3!=mv_index.size()) meshUtils::genFNormals(*this); // calculate per face normals

	if(sp_renderer && !buildEdgeList()) // do this before duplicating vertices due to vertex normals
		m_flags&= (~FLAG_SHADOW);
	if(mv_normal.size()<mv_coord.size()) // are normals already defined?
		meshUtils::genVNormals(*this, 60.0f); // if not, calculate per vertex normals // FIXME: make this factor accessible, dependent on model definition
	if(mv_texCoord.size()<mv_coord.size()) // generate texture coordinates
		meshUtils::genTexCoords(*this,m_mat.texScale());
	if(m_mat.transparent()) m_flags|=FLAG_TRANSPARENT;
}

void proMesh::draw(proCamera & camera) {
    if(!(m_flags&FLAG_ACTIVE)||!(m_flags&FLAG_RENDER)) return;
    if(camera.flags()&FLAG_RENDER) { // normal draw:
        if((m_flags&FLAG_UPDATE) && (m_flags&FLAG_SHADOW)) {
            mv_shadow.clear();
            mv_cap.clear();
            m_flags-=FLAG_UPDATE;
        }
        if(!testBounding(camera.frs(),camera.matrix()))
            return;
	}        
    if((camera.flags()&FLAG_SHADOW)&&camera.light()&&(m_flags&FLAG_SHADOW)&&mv_edge.size()) { // recalculate shadow volumes:
        if((m_bndSphere.radius()<0.0f)||(camera.light()->range()<0.0f)||(m_bndSphere.sqrDistTo(camera.light()->pos())<=camera.light()->range()*camera.light()->range())) { 
			if(camera.light()->flags()&FLAG_UPDATE) {
				mv_shadow.clear();
				mv_cap.clear();
			}
			if(!mv_shadow.size()) { // calculate shadow volume:
				// build list of dot products indicating whether face is pointing away from light source (vDot>0.0) or not
				vector<float> vDot;
				vDot.reserve(mv_fNormal.size());
				float length=(camera.light()->range()>=0.0f) ? camera.light()->range() : 100.0f;

				if(camera.light()->pos()[3]==0.0f) { // distant directional light
					vec3f dir(camera.light()->pos()*-length);
					for(size_t i=0; i<mv_fNormal.size(); ++i) {
						float dot=mv_fNormal[i]*dir;
						vDot.push_back(dot);
						if(dot<=0.0f) { // cap detected
							mv_cap.push_back(mv_coord[mv_index[i*3]]);
							mv_cap.push_back(mv_coord[mv_index[i*3+1]]);
							mv_cap.push_back(mv_coord[mv_index[i*3+2]]);
							mv_cap.push_back(mv_coord[mv_index[i*3]]+dir);
							mv_cap.push_back(mv_coord[mv_index[i*3+2]]+dir);
							mv_cap.push_back(mv_coord[mv_index[i*3+1]]+dir);
						}
					}
					// traverse edge list and store edges that have adjacent normals of opposite dot products:
					for(size_t i=0; i<mv_edge.size(); ++i) 
						if(vDot[mv_edge[i].normalIndex[0]]*vDot[mv_edge[i].normalIndex[1]]<=0.0f) {
							if(vDot[mv_edge[i].normalIndex[0]]<=0.0f) {
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[1]]);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[0]]);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[0]]+dir);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[1]]+dir);
							}
							else {
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[0]]);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[1]]);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[1]]+dir);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[0]]+dir);
							}
						}
				}
				else { // point light
					for(size_t i=0; i<mv_fNormal.size(); ++i) {
						vec3f dir0(camera.light()->pos(),mv_coord[mv_index[i*3]]);
						float dot=mv_fNormal[i]*dir0;
						vDot.push_back(dot);
						if(dot<=0.0f) { // cap detected
							dir0.normalize();
							dir0*=length;
							vec3f dir1(camera.light()->pos(),mv_coord[mv_index[i*3+1]]);
							dir1.normalize();
							dir1*=length;
							vec3f dir2(camera.light()->pos(),mv_coord[mv_index[i*3+2]]);
							dir2.normalize();
							dir2*=length;
							mv_cap.push_back(mv_coord[mv_index[i*3]]);
							mv_cap.push_back(mv_coord[mv_index[i*3+1]]);
							mv_cap.push_back(mv_coord[mv_index[i*3+2]]);
							mv_cap.push_back(mv_coord[mv_index[i*3]]+dir0);
							mv_cap.push_back(mv_coord[mv_index[i*3+2]]+dir2);
							mv_cap.push_back(mv_coord[mv_index[i*3+1]]+dir1);
						}
					}
					// traverse edge list and store edges that have adjacent normals of opposite dot products:
					for(size_t i=0; i<mv_edge.size(); ++i) 
						if(vDot[mv_edge[i].normalIndex[0]]*vDot[mv_edge[i].normalIndex[1]]<=0.0f) {
							vec3f dir0(camera.light()->pos(),mv_coord[mv_edge[i].vertexIndex[0]]);
							dir0.normalize();
							dir0*=length;
							vec3f dir1(camera.light()->pos(),mv_coord[mv_edge[i].vertexIndex[1]]);
							dir1.normalize();
							dir1*=length;
							if(vDot[mv_edge[i].normalIndex[0]]<=0.0f) {
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[1]]);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[0]]);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[0]]+dir0);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[1]]+dir1);
							}
							else {
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[0]]);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[1]]);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[1]]+dir1);
								mv_shadow.push_back(mv_coord[mv_edge[i].vertexIndex[0]]+dir0);
							}
						}
				}
			}
        }
	}
	
	if(mp_renderable) mp_renderable->draw(camera);
}

void proMesh::calcBounding(bool) {
    if(!mv_coord.size()) return;
    // calculate bounding box parallel to coordinate system:
    m_bbox.first = m_bbox.second = mv_coord[0];
    size_t i;
    for(i=1; i<mv_coord.size(); ++i) {
        if(mv_coord[i][X]<m_bbox.first[X]) m_bbox.first[X]=mv_coord[i][X];
        else if(mv_coord[i][X]>m_bbox.second[X]) m_bbox.second[X]=mv_coord[i][X];
        if(mv_coord[i][Y]<m_bbox.first[Y]) m_bbox.first[Y]=mv_coord[i][Y];
        else if(mv_coord[i][Y]>m_bbox.second[Y]) m_bbox.second[Y]=mv_coord[i][Y];
        if(mv_coord[i][Z]<m_bbox.first[Z]) m_bbox.first[Z]=mv_coord[i][Z];
        else if(mv_coord[i][Z]>m_bbox.second[Z]) m_bbox.second[Z]=mv_coord[i][Z];
    }
    // search for minimal radius from center of bounding box:
    vec3f c((m_bbox.first+m_bbox.second)*0.5f);
    float rSquared=0.0f;
    for(i=0; i<mv_coord.size(); ++i) {
        float currDist=c.sqrDistTo(mv_coord[i]);
        if(currDist>rSquared) rSquared=currDist;
    }
    m_bndSphere=sphere(c,sqrt(rSquared));
}

void proMesh::transform(const mat4f & m) { 
	m.transform(mv_coord); 
	mat4f mTrInv(m.inverse()); // transposed inverse matrix for normals
	mTrInv.transpose();
	if(mv_normal.size()) {
		mTrInv.transform(mv_normal);
		for(vector<vec3f>::iterator it=mv_normal.begin(); it!=mv_normal.end(); ++it)
			it->normalize();
	}
	if(mv_fNormal.size()) {
		mTrInv.transform(mv_fNormal);
		for(vector<vec3f>::iterator it=mv_fNormal.begin(); it!=mv_fNormal.end(); ++it)
			it->normalize();
	}
	calcBounding(); 
}

Xml proMesh::xml() const {
    Xml shape("Shape");
    if(m_name.size()) shape.attr("DEF",m_name);
    shape.append(m_mat.xml());
	//shape.child(0).first->attr("DEF",""); // avoid non-unique DEFs
	//Xml * pImageTexture = shape.child(0).first->child("ImageTexture","texScale");
	//if(pImageTexture) pImageTexture->attr("texScale","");
    Xml indfs("IndexedFaceSet");
    string ci;
    for(size_t i=0; i+2<mv_index.size(); i+=3)
        ci+=i2s(mv_index[i])+' '+i2s(mv_index[i+1])+' '+i2s(mv_index[i+2])+" -1, ";
    indfs.attr("coordIndex",ci);
    Xml coord("Coordinate");
    string pt;
    for(size_t i=0; i<mv_coord.size(); ++i)
        pt+=f2s(mv_coord[i][X])+' '+f2s(mv_coord[i][Z])+' '+f2s(-mv_coord[i][Y])+", ";
    coord.attr("point",pt);
    indfs.append(coord);
    if(mv_texCoord.size()) {
        Xml tcoord("TextureCoordinate");
        tcoord.attr("point",join(mv_texCoord));
        indfs.append(tcoord);
    }
    if(mv_normal.size()) {
        indfs.attr("normalPerVertex","TRUE");
        Xml ncoord("Normal");
        pt.erase();
        for(size_t i=0; i<mv_normal.size(); ++i)
            pt+=f2s(mv_normal[i][X])+' '+f2s(mv_normal[i][Z])+' '+f2s(-mv_normal[i][Y])+", ";
        ncoord.attr("vector",pt);
        indfs.append(ncoord);
    }
    if(mv_color.size()) {
        indfs.attr("colorPerVertex","TRUE");
        Xml ccoord("Color");
        ccoord.attr("color",join(mv_color));
        indfs.append(ccoord);
    }
	if(m_flags&FLAG_FRONT_AND_BACK)
		indfs.attr("solid","FALSE");
    
    shape.append(indfs);
    return shape;
}

bool proMesh::buildEdgeList() {
	// first build an index list without duplicated vertices:
	vector<unsigned int> vIndex(mv_index);
	for(size_t i=0; i<vIndex.size(); ++i)
		for(size_t j=0; j<i; ++j) if(mv_coord[vIndex[j]]==mv_coord[vIndex[i]]) {
			vIndex[i]=vIndex[j];
			break;
		}
	mv_edge.reserve(vIndex.size()); // allocate enough space to hold all edges
	for (size_t a = 0; a+2 < vIndex.size(); a+=3) { // first pass: find edges
		size_t i1 = vIndex[a];
		size_t i2 = vIndex[a+1];
		size_t i3 = vIndex[a+2];

		if (i1 < i2)
            mv_edge.push_back(edge(i1, i2, a/3, UINT_MAX));
		if (i2 < i3) 
            mv_edge.push_back(edge(i2, i3, a/3, UINT_MAX));
		if (i3 < i1)
            mv_edge.push_back(edge(i3, i1, a/3, UINT_MAX));
	}

	// second pass: match triangles to edges
	for (size_t a = 0; a+2 < vIndex.size(); a+=3) {
		size_t i1 = vIndex[a];
		size_t i2 = vIndex[a+1];
		size_t i3 = vIndex[a+2];
		
		if (i1 > i2) for(size_t b = 0; b < mv_edge.size(); ++b)
            if ((mv_edge[b].vertexIndex[0] == i2) &&
                (mv_edge[b].vertexIndex[1] == i1) &&
                (mv_edge[b].normalIndex[1] == UINT_MAX)) {
                mv_edge[b].normalIndex[1] = a/3;
                break;
            }
		
		if (i2 > i3) for(size_t b = 0; b < mv_edge.size(); ++b)
            if ((mv_edge[b].vertexIndex[0] == i3) &&
                (mv_edge[b].vertexIndex[1] == i2) &&
                (mv_edge[b].normalIndex[1] == UINT_MAX)) {
                mv_edge[b].normalIndex[1] = a/3;
                break;
            }
		
		if (i3 > i1) for(size_t b = 0; b < mv_edge.size(); ++b)
            if ((mv_edge[b].vertexIndex[0] == i1) &&
                (mv_edge[b].vertexIndex[1] == i3) &&
                (mv_edge[b].normalIndex[1] == UINT_MAX)) {
                mv_edge[b].normalIndex[1] = a/3;
                break;
            }
	}
	// final pass: check that all edges normals are assigned, otherwise clear edge list (no shadows):
	for(size_t i=0; i<mv_edge.size(); ++i) 
		if((mv_edge[i].normalIndex[0]==UINT_MAX)||(mv_edge[i].normalIndex[1]==UINT_MAX)) {
			mv_edge.clear();
			return false;
		}
	return true;
}

bool proMesh::intersects(const line & ray) const {
    // first test on bounding level:
    if(!ray.intersects(m_bndSphere)) return false;
    // now test on individual triangles:
    for(size_t i=0; i+2<mv_index.size(); i+=3)
        if(ray.intersects(mv_coord[mv_index[i]],mv_coord[mv_index[i+1]],mv_coord[mv_index[i+2]])) 
            return true;
    return false;
}

vec3f * proMesh::intersection(const line & ray) const {
    // first test on bounding level:
    if(!ray.intersects(m_bndSphere)) return 0;
    // now test on individual triangles:
    vec3f dir(ray[0],ray[1]);
    float minDist=FLT_MAX;
    for(size_t i=0; i+2<mv_index.size(); i+=3) {
        const vec3f & tr0=mv_coord[mv_index[i]];
        const vec3f & tr1=mv_coord[mv_index[i+1]];
        const vec3f & tr2=mv_coord[mv_index[i+2]];
        // is dir parallel to tr?:
        vec3f edge1(tr0,tr1);
        vec3f edge2(tr0,tr2);
        vec3f pvec(dir.crossProduct(edge2));
        float det = edge1*pvec;
        if(det > -EPSILON && det < EPSILON) continue;
        float invDet = 1.0f/det;
    
        // is intersection within triangle?:
        vec3f tvec(tr0,ray[0]);
        float u = (tvec*pvec) * invDet;
        if(u < 0.0f || u > 1.0f) continue;
        vec3f qvec(tvec.crossProduct(edge1));
        float v = (dir*qvec) * invDet;
        if(v < 0.0f || (u + v) > 1.0f) continue;
    
        // compute intersection distance from pt[0]:
        float currDist = (edge2*qvec) * invDet;
        if((currDist >= 0.0f)&&(currDist<minDist)) minDist=currDist;
    }
    if(minDist<FLT_MAX) { // compute intersection vertex:
        vec3f * pResult=new vec3f(ray[0]);
        pResult->translate(dir,minDist);
        return pResult;
    }
    return 0;
}

void proMesh::addFace(const vec3f & vtx0, const vec3f & vtx1, const vec3f & vtx2) {
    // store vertex pointers:
    unsigned int vt0Idx=mv_coord.size()+4;
    unsigned int vt1Idx=vt0Idx;
    unsigned int vt2Idx=vt0Idx;
    unsigned int i;
    for(i=0; i<mv_coord.size(); ++i) {
        if(mv_coord[i]==vtx0) vt0Idx=i;
        if(mv_coord[i]==vtx1) vt1Idx=i;
        if(mv_coord[i]==vtx2) vt2Idx=i;
    }
    if(vt0Idx>mv_coord.size()) {
        mv_coord.push_back(vtx0);
        vt0Idx=mv_coord.size()-1;
    }
    if(vt1Idx>mv_coord.size()) {
        mv_coord.push_back(vtx1);
        vt1Idx=mv_coord.size()-1;
    }
    if(vt2Idx>mv_coord.size()) {
        mv_coord.push_back(vtx2);
        vt2Idx=mv_coord.size()-1;
    }
    mv_index.push_back(vt0Idx);
    mv_index.push_back(vt1Idx);
    mv_index.push_back(vt2Idx);
}
