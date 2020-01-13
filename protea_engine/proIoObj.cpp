#include "proIoObj.h"
#include <protea.h>
#include <fstream>

using namespace std;

//--- loading ------------------------------------------------------

static int loadMtl(const std::string & filename) {
    //cout << "loadMtl()" << endl;
    ifstream file(filename.c_str(), std::ios::in);
    if (!file.good()) {
        cerr << "ioObj::loadMtl() ERROR: " << filename << " file error or file not found!\n";
        return 1;
    }
    
    proMaterial* pMtl=0;
    string line;
    while(!file.eof()) {
        getline(file,line);
        line=trim(line);
        if(!line.size()||(line[0]=='#')) continue;
        vector<string> vWord;
        split(line,vWord);
        
        if(vWord.size()<2) continue;
        if(vWord[0]=="newmtl") {
            if(pMtl) {
                MaterialMgr::singleton().add(*pMtl);
                delete pMtl;
				pMtl = 0;
            }
            pMtl= new proMaterial(vWord[1]);
            continue;
        }
        if(!pMtl) continue;
        if(vWord[0]=="Ns")
            pMtl->shininess(s2f(vWord[1]));
        else if((vWord[0]=="Ka")&&(vWord.size()>3))
            pMtl->ambientColor(vec3f(s2f(vWord[1]),s2f(vWord[2]),s2f(vWord[3])));
        else if((vWord[0]=="Kd")&&(vWord.size()>3))
            pMtl->color(s2f(vWord[1]),s2f(vWord[2]),s2f(vWord[3]),pMtl->color()[3]);
        else if((vWord[0]=="Ks")&&(vWord.size()>3))
            pMtl->specularColor(vec3f(s2f(vWord[1]),s2f(vWord[2]),s2f(vWord[3])));
        else if((vWord[0]=="d")||(vWord[0]=="Tr")) {
			vec4f col(pMtl->color());
            col[3]=s2f(vWord[1]);
			pMtl->color(col);
		}
        else if(vWord[0].find("map")<vWord[0].size()) 
            pMtl->texName(vWord[1]);
    }
    file.close();
    if(pMtl) MaterialMgr::singleton().add(*pMtl);
    return 0;
}

proNode * ioObj::load(const std::string & filename) {
    //cout << "ioObj::load" << endl;
    string fname=io::unifyPath(filename);
    unsigned int filenamePos=fname.rfind('/');
    ifstream file(fname.c_str(), std::ios::in);
    if (!file.good()) {
        cerr << "ioObj::load() ERROR: " << fname << " file error or file not found!\n";
        return 0;
    }
    
    proTransform * pParent=new proTransform(filename);
    
    string line;
    proMesh * pMesh=new proMesh;
    bool faceMode=false;
    unsigned int vIndexOffset=1;
    unsigned int tIndexOffset=1;
    unsigned int nIndexOffset=1;
    // storage for further temporary indices:
    vector<size_t> vTexIndices;
    vector<size_t> vNormalIndices;
    vector<size_t> vFaceEnds;
        
    while(!file.eof()) {
        getline(file,line);
        line=trim(line);
        vector<string> vWord;
        split(line,vWord);
        
        if(vWord.size()&&(vWord[0]=="f")) {
            faceMode=true;
            if(vWord.size()>3) {
                for(unsigned int i=1; i<vWord.size(); ++i) {
                    unsigned int slashPos=vWord[i].find('/');
                    int index=s2i(vWord[i].substr(0,slashPos));
                    if(index<0) index+=pMesh->indices().size();
                    else index-=vIndexOffset;
                    pMesh->indices().push_back(index);
                    //cout << "v:" << index << " ";
                    if(slashPos<vWord[i].size()) {
                        index=s2i(vWord[i].substr(slashPos+1,vWord[i].rfind('/')+1-slashPos));
                        if(index<0) vTexIndices.push_back(vTexIndices.size()+index);
                        else if(index>0) vTexIndices.push_back(index-tIndexOffset);
                        //if(index) cout << "t:" << index-tIndexOffset << " ";
                        
                        if(vWord[i].find('/',slashPos+1)<vWord[i].size()) {
                            index=s2i(vWord[i].substr(vWord[i].rfind('/')+1,vWord[i].size()));
                            if(index<0) vNormalIndices.push_back(vNormalIndices.size()+index);
                            else if(index>0) vNormalIndices.push_back(index-nIndexOffset);
                            //if(index) cout << "n:" << index-nIndexOffset;
                        }
                    }
                    //cout << endl;
                }
                vFaceEnds.push_back(pMesh->indices().size()-1);
            }
            continue;
        }
        else if(faceMode) { // mesh is over
			// normalize between various indices:
			if(vTexIndices.size()&&(vTexIndices.size()!=pMesh->indices().size()))
				vTexIndices.clear();
			if(vNormalIndices.size()&&(vNormalIndices.size()!=pMesh->indices().size()))
				vNormalIndices.clear();
			if(vTexIndices.size()||vNormalIndices.size()) {
				vector<vec3f> vCoord;
				vCoord.reserve(pMesh->indices().size());
				vector<vec2f> vTexCoord;
				vTexCoord.reserve(pMesh->indices().size());
				vector<vec3f> vNormal;
				vNormal.reserve(pMesh->indices().size());
				vector<unsigned int> vIndex;
				vIndex.reserve(pMesh->indices().size());
				for(size_t i=0; i<pMesh->indices().size(); ++i) { // normalize indices:
					// search for suitable vertex:
					size_t index=UINT_MAX;
					for(size_t j=0; j<vCoord.size(); ++j) {
						if(vCoord[j]!=pMesh->coords()[pMesh->indices()[i]]) 
							continue;
						if(pMesh->texCoords().size()&&(vTexCoord[j]!=pMesh->texCoords()[vTexIndices.size() ? vTexIndices[i] : pMesh->indices()[i]]))
							continue;
						if(pMesh->vNormals().size()&&(vNormal[j]!=pMesh->vNormals()[vNormalIndices.size() ? vNormalIndices[i] : pMesh->indices()[i]]))
							continue;
						index=j;
						break;
					}
					if(index==UINT_MAX) { // no suitable vertex found, add new:
						index=static_cast<unsigned int>(vCoord.size());
						vCoord.push_back(pMesh->coords()[pMesh->indices()[i]]);
						if(pMesh->texCoords().size())
							vTexCoord.push_back(pMesh->texCoords()[vTexIndices.size() ? vTexIndices[i] : pMesh->indices()[i]]);
						if(pMesh->vNormals().size())
							vNormal.push_back(pMesh->vNormals()[vNormalIndices.size() ? vNormalIndices[i] : pMesh->indices()[i]]);
					}
					vIndex.push_back(index);
				}
				pMesh->coords()=vCoord;
				if(pMesh->texCoords().size()) pMesh->texCoords()=vTexCoord;
				if(pMesh->vNormals().size()) pMesh->vNormals()=vNormal;
				pMesh->indices()=vIndex;
			}
			// triangulate vertex indices based on face ends:
			vector<unsigned int> vIndex;
			// rebuild indices list:
			size_t currStart=0;
			size_t currEnd=0;
			for(size_t i=0; (i<pMesh->indices().size())&&(currEnd<vFaceEnds.size()); ++i) {
				if(i>currStart+1) {
					vIndex.push_back(pMesh->indices()[currStart]);
					vIndex.push_back(pMesh->indices()[i-1]);
					vIndex.push_back(pMesh->indices()[i]);
				}
				if(i==vFaceEnds[currEnd]) {
					++currEnd;
					currStart=i+1;
				}
			}
			pMesh->indices()=vIndex;
			
			// append:
            pParent->append(pMesh,false);
			// cleanup for next mesh:
            vIndexOffset+=pMesh->coords().size();
            tIndexOffset+=pMesh->texCoords().size();
            nIndexOffset+=pMesh->vNormals().size();
			vTexIndices.clear();
			vNormalIndices.clear();
			vFaceEnds.clear();
            pMesh=new proMesh;
            faceMode=false;
        }
        if(!line.size()||(line[0]=='#')) continue;
            
        if(((vWord[0]=="g")||(vWord[0]=="o"))&&(vWord.size()>1))
            pMesh->name(vWord[1]);
        else if((vWord[0]=="v")&&(vWord.size()>3))
            pMesh->coords().push_back(vec3f(s2f(vWord[1]),s2f(vWord[2]),s2f(vWord[3])));
        else if((vWord[0]=="vt")&&(vWord.size()>2))
            pMesh->texCoords().push_back(vec2f(s2f(vWord[1]),s2f(vWord[2])));
        else if((vWord[0]=="vn")&&(vWord.size()>3))
            pMesh->vNormals().push_back(vec3f(s2f(vWord[1]),s2f(vWord[2]),s2f(vWord[3])));
        else if((vWord[0]=="usemtl")&&(vWord.size()>1))
			pMesh->material(MaterialMgr::singleton()[vWord[1]]);
        else if(vWord[0]=="mtllib") for(unsigned int i=1; i<vWord.size();++i) {
            if(filenamePos<fname.size())
                loadMtl(fname.substr(0,filenamePos+1)+vWord[i]);
            else loadMtl(vWord[i]);
        }
    }
    file.close();
    delete pMesh;
    //cout << "pParent->size():" << pParent->size() << endl;
    if(pParent->size()==1) {
        pMesh=new proMesh(*static_cast<proMesh*>((*pParent)[0]));
        delete pParent;
        return pMesh;
    }
    else if(pParent->size()) return pParent;
    delete pParent;
    return 0;
}


//--- saving -------------------------------------------------------

int ioObj::save(const proNode & model, const std::string & filename) {
    // first flatten all transforms:
    proNode* pModel=const_cast<proNode*>(&model)->copy();
    meshUtils::flattenTransforms(*pModel);

    // collect geometry and material:
    string mtlFileName=filename.substr(0,filename.rfind('.')+1)+"mtl";
    string obj("# "+filename+"\nmtllib "+mtlFileName+"\n\n");

	// traverse scene graph and convert meshes to obj:
    unsigned int vIndexOffset=1;
    unsigned int tIndexOffset=1;
    unsigned int nIndexOffset=1;
	for(proNode::iterator it = pModel; it!=0; ++it)  if(it->type()==proMesh::TYPE) {
        proMesh & mesh=*dynamic_cast<proMesh*>(*it);
        // add material data:
        if(mesh.name().size())
            obj+="o "+mesh.name()+'\n';
        else obj+="o mesh"+mesh.name()+'\n';
        obj+="usemtl "+mesh.material().name()+'\n';
        // add vertex data:
        for(unsigned int i=0; i<mesh.coords().size(); ++i)
            obj+="v "+f2s(mesh.coords()[i][X])+' '+f2s(mesh.coords()[i][Y])+' '+f2s(mesh.coords()[i][Z])+'\n';
        for(unsigned int i=0; i<mesh.texCoords().size(); ++i)
            obj+="vt "+f2s(mesh.texCoords()[i][X])+' '+f2s(mesh.texCoords()[i][Y])+'\n';
        for(unsigned int i=0; i<mesh.vNormals().size(); ++i)
            obj+="vn "+f2s(mesh.vNormals()[i][X])+' '+f2s(mesh.vNormals()[i][Y])+' '+f2s(mesh.vNormals()[i][Z])+'\n';
        // add indices:
        if(mesh.indices().size()>2) {
            obj+="f";
            for(unsigned int i=0;i<mesh.indices().size(); ++i) {
                obj+=" "+i2s(mesh.indices()[i]+vIndexOffset);
                if(i%3==2) {
                    if(i+1<mesh.indices().size()) obj+="\nf";
                    else obj+="\n\n";
                }
            }
        }
        vIndexOffset+=mesh.coords().size();
        tIndexOffset+=mesh.texCoords().size();
        nIndexOffset+=mesh.vNormals().size();
    }
    
    // save OBJ file:
    ofstream file(filename.c_str(), std::ios::out);
    if (!file.good()) {
        cerr << "ioObj::save() ERROR: " << filename << " file error.\n";
        return 1;
    }
    file << obj << flush;
    file.close();
    
    // generate and save MTL file:    
    file.open(mtlFileName.c_str(), std::ios::out);
    if (!file.good()) {
        cerr << "ioObj::save() ERROR: " << mtlFileName << " file error.\n";
        return 1;
    }
    MaterialMgr & matTable=MaterialMgr::singleton();
    for(unsigned int i=0; i<matTable.size(); ++i) {
        file<< "newmtl " << matTable[i].name() << '\n'
            << "Ns " << matTable[i].shininess() << '\n'
            << "Ka " << matTable[i].ambientColor()[0] << ' ' << matTable[i].ambientColor()[1]
                << ' ' << matTable[i].ambientColor()[2] << '\n'
            << "Kd " << matTable[i].color()[0] << ' ' << matTable[i].color()[1] 
                << ' ' << matTable[i].color()[2] << '\n'
            << "Ks " << matTable[i].specularColor()[0] << ' ' << matTable[i].specularColor()[1] 
                << ' ' << matTable[i].specularColor()[2] << '\n'
            << "d " << matTable[i].color()[3] << '\n'
            << "illum 2\n";
        if(matTable[i].texName().size())
             file << "map_Kd " << matTable[i].texName() << "\n\n" << flush;
        else file << endl;
    }
    
    file.close();    
    return 0;
}
