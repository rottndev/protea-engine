#include "proMesh.h"
#include "proScene.h"
#include "proMath.h"
#include "proStr.h"
#include "proIo.h"
#include "proResource.h"
#include <map>
#include <fstream>
using namespace std;

/// recursively flattens scene graph
void meshUtils::flattenHierarchy(proTransform & parent, proNode & node) {
    if((node.type()!=proTransform::TYPE)&&(node.type()!=proScene::TYPE)) parent.append(&node);
    else for(unsigned int i=0; i<static_cast<const proTransform*>(&node)->size(); ++i)
        flattenHierarchy(parent,*static_cast<const proTransform*>(&node)->operator[](i));
}

/// remove transformations from the scene:
void meshUtils::flattenTransforms(proNode & node) {
    if((node.type()==proTransform::TYPE)||(node.type()==proScene::TYPE)) {
        proTransform & transf=*((proTransform*)&node);
        for(unsigned int i=0; i<transf.size(); ++i)
            flattenTransforms(*transf[i]);
		if(!transf.matrix().isIdentity()) {
            transf.transform(transf.matrix());
			transf.reset();
		}
    }
}

void meshUtils::genFNormals(proMesh & m) {
    m.fNormals().clear();
    m.fNormals().reserve(m.indices().size()/3);
	for(unsigned int i=0; i+2<m.indices().size(); ++i) {
		const vec3f & p0(m.coords()[m.indices()[i]]); ++i;
		const vec3f & p1(m.coords()[m.indices()[i]]); ++i;
		const vec3f & p2(m.coords()[m.indices()[i]]);
		vec3f v1(p0,p1);
		vec3f v2(p0,p2);
        // compute normal:
		vec3f normal=v1.crossProduct(v2);
		if(isnan(normal[X])||isnan(normal[Y])||isnan(normal[Z])||(normal.sqrLength()==0.0f))
			normal.set(0.0f,0.0f,1.0f);
		else normal.normalize();
		m.fNormals().push_back(normal);
	}
}

void meshUtils::genVNormals(proMesh & m, float creaseAngle) {
    // calculate per face normals if missing:
    if(m.fNormals().size()!=m.indices().size()/3)
        genFNormals(m);

    // first build a list associating each vertex to its related indices:
    map<unsigned int, vector<unsigned int> > mVtxIndices;
	unsigned int i;
    for(i=0; i<m.indices().size(); ++i) {
        map<unsigned int, vector<unsigned int> >::iterator it=mVtxIndices.find(m.indices()[i]);
        if(it==mVtxIndices.end()) {
            vector<unsigned int> vUI;
            vUI.push_back(i);
            mVtxIndices.insert(make_pair(m.indices()[i],vUI));
        }
        else it->second.push_back(i);
    }
    // then check which vertices need to be duplicated:
    float minScalarProd=static_cast<float>(dcos(creaseAngle));
    for(map<unsigned int, vector<unsigned int> >::iterator it=mVtxIndices.begin(); it!=mVtxIndices.end(); ++it) {
        if(it->second.size()<2) continue;
        vector<pair<unsigned int, vec3f> > vNewVtx;
        for(unsigned int j=0; j+1<it->second.size(); ++j) for(unsigned int k=j+1; k<it->second.size(); ++k) {
            if(m.fNormals()[it->second[j]/3]*m.fNormals()[it->second[k]/3]<minScalarProd) { // sigh, new vertex needed
                bool suitableVtxFound=false;
                for(unsigned int l=0; l<vNewVtx.size(); ++l)
                    if(m.fNormals()[it->second[k]/3]*vNewVtx[l].second>=minScalarProd) {
                        m.indices()[it->second[k]]=vNewVtx[l].first;
                        suitableVtxFound=true;
                        break;
                    }
                if(!suitableVtxFound) { // duplicate vertex:
                    m.coords().push_back(m.coords()[m.indices()[it->second[k]]]);
                    if(m.texCoords().size()) 
                        m.texCoords().push_back(m.texCoords()[m.indices()[it->second[k]]]);
                    if(m.vertexColors().size()) 
                        m.vertexColors().push_back(m.vertexColors()[m.indices()[it->second[k]]]);                
                    m.indices()[it->second[k]]=static_cast<unsigned int>(m.coords().size()-1);
                    vNewVtx.push_back(make_pair(m.indices()[it->second[k]],m.fNormals()[it->second[k]/3]));
                }
            }
        }
    }

    // compute per vertex normals:
    m.vNormals().assign(m.coords().size(),vec3f(0,0,0));
    for(i=0; i<m.indices().size(); ++i)
        m.vNormals()[m.indices()[i]]+=m.fNormals()[i/3];
    for(i=0; i<m.vNormals().size(); ++i) // normalize normals:
        if(!m.vNormals()[i].sqrLength()) m.vNormals()[i].set(0.0f,0.0f,1.0f);
        else m.vNormals()[i].normalize();
}

/// generate texture coordinates:
void meshUtils::genTexCoords(proMesh & m, const vec2f & texScale) {
    if(m.fNormals().size()!=m.indices().size()/3) genFNormals(m);
    m.texCoords().clear();
    m.texCoords().assign(m.coords().size(),vec2f());
	
    for(unsigned int i=0; i<m.indices().size(); ++i) if(fabs(m.fNormals()[i/3][Z])>.71) // always use flat texture coords for (nearly) horizontal faces
        m.texCoords()[m.indices()[i]].set(m.coords()[m.indices()[i]][X]/texScale[X], m.coords()[m.indices()[i]][Y]/texScale[Y]);
    else if((m.vNormals()[m.indices()[i]]==m.fNormals()[i/3]) && (m.fNormals()[i/3][X]||m.fNormals()[i/3][Y])) { // use flat texture coords for flat faces, maybe allow some tolerance instead of == ?
        vec2f texS(-m.fNormals()[i/3][Y],m.fNormals()[i/3][X]);
        texS.normalize();
        texS/=texScale[X];
        m.texCoords()[m.indices()[i]].set(m.coords()[m.indices()[i]][X]*texS[X]+m.coords()[m.indices()[i]][Y]*texS[Y], m.coords()[m.indices()[i]][Z]/texScale[Y]);
    }
    else  if(fabs(m.fNormals()[i/3][Y])>.71) // cube mapping for smooth vertices
        m.texCoords()[m.indices()[i]].set(m.coords()[m.indices()[i]][X]/texScale[X], m.coords()[m.indices()[i]][Z]/texScale[Y]);
	else
        m.texCoords()[m.indices()[i]].set(m.coords()[m.indices()[i]][Y]/texScale[X], m.coords()[m.indices()[i]][Z]/texScale[Y]);
}

void meshUtils::zup2yup(proMesh & m) {
    float f;
    for(unsigned i=0; i<m.coords().size(); ++i) {
        f=m.coords()[i][Y];
        m.coords()[i][Y]=-m.coords()[i][Z];
        m.coords()[i][Z]=f;
    }
    for(unsigned i=0; i<m.fNormals().size(); ++i) {
        f=m.fNormals()[i][Y];
        m.fNormals()[i][Y]=-m.fNormals()[i][Z];
        m.fNormals()[i][Z]=f;
    }
    for(unsigned i=0; i<m.vNormals().size(); ++i) {
        f=m.vNormals()[i][Y];
        m.vNormals()[i][Y]=-m.vNormals()[i][Z];
        m.vNormals()[i][Z]=f;
    }
}

void meshUtils::yup2zup(proMesh & m) {
    float f;
    for(unsigned i=0; i<m.coords().size(); ++i) {
        f=m.coords()[i][Y];
        m.coords()[i][Y]=m.coords()[i][Z];
        m.coords()[i][Z]=-f;
    }
    for(unsigned i=0; i<m.fNormals().size(); ++i) {
        f=m.fNormals()[i][Y];
        m.fNormals()[i][Y]=m.fNormals()[i][Z];
        m.fNormals()[i][Z]=-f;
    }
    for(unsigned i=0; i<m.vNormals().size(); ++i) {
        f=m.vNormals()[i][Y];
        m.vNormals()[i][Y]=m.vNormals()[i][Z];
        m.vNormals()[i][Z]=-f;
    }
}

//--- subdivision functions ----------------------------------------

/// a little internal helper class that stores indices of vertex pairs and their center for subdividing
class index3 {
public:
    index3(unsigned int index0, unsigned int index1, unsigned int index2)
        : iMin(min(index0,index1)), iMax(max(index0,index1)), iCtr(index2) { }
    unsigned int iMin;
    unsigned int iMax;
    unsigned int iCtr;
    //std::string str() const { return i2s(iMin)+' '+i2s(iMax)+' '+i2s(iCtr); }
};


unsigned int meshUtils::subdivide(proMesh & mesh, float maxDist) {
    // mesh must already be triangulated!
    float sqrMaxDist=maxDist*maxDist;

    vector<index3> vCenter; // stores already calculated centers between two indices for reuse
    vector<index3> vTexCenter; // stores already calculated centers between two texture indices for reuse
    unsigned int j=0;
    while(j<mesh.indices().size()) {
        unsigned int index[3];
        index[0]=mesh.indices()[(j/3)*3];
        index[1]=mesh.indices()[(j/3)*3+1];
        index[2]=mesh.indices()[(j/3)*3+2];

        //cout << j << " " << mesh.indices().size() << " ";
        unsigned int i;
        float d[3];
        for(i=0; i<3; ++i)
            d[i]=mesh.coords()[index[i]].sqrDistTo(mesh.coords()[index[(i+1)%3]]);
        i=0;
        if(d[1]>d[i]) i=1;
        if(d[2]>d[i]) i=2;

        //cout << d[i] << " " << sqrMaxDist << endl;
        if(d[i]<=sqrMaxDist) {
            j+=3;
            continue;
        }
        // look whether texture coordinater has already been calculated:
        vector<index3>::iterator iter;
        unsigned int iCenter, iTexCenter;

        // look whether center has already been calculated:
        unsigned int iMin=min(index[i],index[(i+1)%3]);
        unsigned int iMax=max(index[i],index[(i+1)%3]);
        for(iter=vCenter.begin(); iter!=vCenter.end(); ++iter)
            if((iter->iMin==iMin)&&(iter->iMax==iMax))
                break;
        if(iter!=vCenter.end()) iCenter=iter->iCtr;
        else { // calculate new center:
            vec3f vtCenter((mesh.coords()[index[i]]+mesh.coords()[index[(i+1)%3]])*0.5f);
            //cout << "vtCenter:" << vtCenter << endl;
            mesh.coords().push_back(vtCenter);
            iCenter=static_cast<unsigned int>(mesh.coords().size()-1);
            vCenter.push_back(index3(index[i], index[(i+1)%3], iCenter));
            if(mesh.texCoords().size()) {
                // interpolate texture coordinate:
                vec2f txCenter((mesh.texCoords()[index[i]]+mesh.texCoords()[index[(i+1)%3]])*0.5f);
                mesh.texCoords().push_back(txCenter);
                iTexCenter=0;
            }
        }
        // divide existing triangle:
        mesh.indices().push_back(index[(i+2)%3]);
        mesh.indices().push_back(index[i]);
        mesh.indices().push_back(iCenter);
        mesh.indices()[j+i]=iCenter;
    }
    // colors and normals are currently not interpolated:
    mesh.vertexColors().clear();
    mesh.vNormals().clear();
    mesh.fNormals().clear();
    //cout << mesh.vrml() << endl;

    return static_cast<unsigned int>(mesh.indices().size()/3);
}

unsigned int meshUtils::subdivide(proNode & node, float maxDist, bool ignoreTransp) {
    unsigned int nTriangles=0;
    if(node.type()==proMesh::TYPE) {
        if(ignoreTransp&&(node.flags()&FLAG_TRANSPARENT))
            nTriangles+=static_cast<unsigned int>(((proMesh*)(&node))->indices().size())/3;
        else nTriangles+=subdivide(*((proMesh*)(&node)),maxDist);
    }
    else if((node.type()==proTransform::TYPE)||(node.type()==proScene::TYPE)) {
        proTransform & transf=*dynamic_cast<proTransform*>(&node);
        if(&transf) for(size_t n=0; n<transf.size(); ++n)
            nTriangles+=subdivide(*(transf[n]),maxDist,ignoreTransp);
    }
    return nTriangles;
}


//--- class ModelMgr --------------------------------------------

ModelMgr* ModelMgr::sp_instance = 0;

ModelMgr::ModelMgr() { 
	loaderRegister(loadRaw,"txt"); 
	loaderRegister(loadRaw,"raw"); 
	loaderRegister(loadX3d,"x3d"); 
	saverRegister(saveX3d,"x3d"); 
}

void ModelMgr::loaderRegister(proNode *(*loadFunc)(const std::string &), const std::string & suffix) {
	mm_loader.insert(make_pair(toLower(suffix),loadFunc)); 
}

void ModelMgr::saverRegister(int(*saveFunc)(const proNode &, const std::string &), const std::string & suffix) {
	mm_saver.insert(make_pair(toLower(suffix),saveFunc)); 
}

bool ModelMgr::loaderAvailable(const std::string & suffix) const {
	return mm_loader.find(toLower(suffix))!=mm_loader.end(); 
}

bool ModelMgr::saverAvailable(const std::string & suffix) const {
	return mm_saver.find(toLower(suffix))!=mm_saver.end(); 
}

proNode * ModelMgr::load(const std::string & filename) {
	if(filename.rfind('.')>filename.size()) return 0;
	string suffix=toLower(filename.substr(filename.rfind('.')+1));
	map<string, proNode * (*)(const string &)>::iterator it = mm_loader.find(suffix);
	if((it==mm_loader.end())||!io::fileExist(filename)) return 0;
	string fname(io::unifyPath(filename));
	if(fname.rfind('/')<fname.size())
	TextureMgr::singleton().searchPathAppend(fname.substr(0,fname.rfind('/')+1));
	return it!=mm_loader.end() ? (*(it->second))(fname) : 0;
}

int ModelMgr::save(const proNode & model, const std::string & filename) {
	if(filename.rfind('.')>filename.size()) return -1;
	string suffix=toLower(filename.substr(filename.rfind('.')+1));
	map<string, int (*)(const proNode &, const string &)>::iterator it = mm_saver.find(suffix);
	if(it!=mm_saver.end()) return (*(it->second))(model,filename);
	return -1;
}

proNode * ModelMgr::loadRaw(const string & filename) {
	if(!io::fileExist(filename)) {
		cerr << "ModelMgr::loadRaw() ERROR: could not find file \"" << filename << "\".\n";
		return 0;
	}
    ifstream file(filename.c_str(), std::ios::in);
    if (!file.good()) {
        cerr << "ModelMgr::loadRaw() ERROR: \"" << filename.c_str() << "\" file error.\n";
        return 0;
    }
	proMesh * pMesh = 0;
	map<vec3f,size_t> mIndex; // for reusing vertices
	
    string currLine;
    while(file.good()) { // main reading loop
        getline(file,currLine);
        if(currLine.size()&&(currLine[0]!='%')&&(currLine[0]!='#')) {
            vector<float> vF;
            s2f(currLine,vF);
            if(vF.size()>8) {
				if(!pMesh) pMesh = new proMesh(filename);
				for(unsigned int i=0; i<3; ++i) {
					vec3f vtx(vF[i*3],vF[i*3+1],vF[i*3+2]);
					map<vec3f,size_t>::iterator it=mIndex.find(vtx);
					if(it!=mIndex.end()) pMesh->indices().push_back(it->second);
					else {
						pMesh->indices().push_back(pMesh->coords().size());
						mIndex.insert(make_pair(vtx,pMesh->coords().size()));
						pMesh->coords().push_back(vtx);
					}
				}
			}
        }
    }
    file.close();
    return pMesh;
}

proNode * ModelMgr::loadX3d(const std::string & filename) {
	return proNode::interpret(Xml::load(filename)); 
}

int ModelMgr::saveX3d(const proNode & model, const std::string & filename) {
	Xml xml("X3D");
	xml.attr("version","3.0");
	xml.attr("profile","Interchange");
	Xml head("head");
	Xml meta("meta");
	meta.attr("name","filename");
	meta.attr("content",filename);
	xml.append(head).append(meta);
	xml.append(model.xml());
	xml.child(xml.nChildren()-1).first->tag("Scene");
	return xml.save(filename);
}


