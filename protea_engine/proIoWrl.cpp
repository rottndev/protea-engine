#include "proIoWrl.h"

#include <proXml.h>
#include <proStr.h>
#include <proScene.h>
#include <iostream>
#include <fstream>

using namespace std;

Xml vrmlToX3d (const vector<string> & token) {
    // prepare x3d root:
    Xml xs("Scene");

    // split into nodes and transform into Xml:
    Xml *currSt = &xs;
    size_t i;
    string s;
    for (i=0; i<token.size(); ++i) {
        if(!token[i].size()) continue;
        //cout << token[i] << " " << flush;
        // common operations:
        if((token[i][0]=='{')&&(i>0)) {
            Xml newSt;
            newSt.tag(token[i-1]);
            if((i>2)&&(token[i-3]=="DEF")) newSt.attr("DEF",token[i-2]);
            //cout << "\nnewSt: " << newSt.str() << flush;
            //cout << "curSt: " << currSt->tag() << endl;
            currSt->append(newSt);
            currSt=currSt->child(currSt->nChildren()-1).first;
            //cout << "curSt after: " << currSt->tag() << endl;
        }
        else if((token[i]=="USE")&&(i>0)&&(i+1<token.size())) {
            string name=token[i-1];
            name[0]=toupper(name[0]);
            Xml xCh(name);
            xCh.attr("USE",token[++i]);
            currSt->append(xCh);
        }
        else if((token[i]=="FALSE")||(token[i]=="TRUE"))
            currSt->attr(token[i-1],token[i]);
        
        else if(token[i][0]=='}') {
            if((currSt->tag()=="TextureCoordinate")||(currSt->tag()=="Coordinate"))
                currSt->attr("point",s);
            else if(currSt->tag()=="Color")
                currSt->attr("color",s);
            else if(currSt->tag()=="Normal")
                currSt->attr("vector",s);
            s.erase();
            currSt=currSt->parent();
            //cout << "curSt back: " << currSt->tag() << endl;
        }
        else if(token[i]=="DEF") ++i;  // skip also following id
        else if((i<token.size()-2)&&(token[i+1][0]=='{')) { } // skip subnode tags
        // special operations for recognized nodes:
        else if(currSt->tag()=="IndexedFaceSet") {
            if((token[i]=="coordIndex")||(token[i]=="texCoordIndex")
               ||(token[i]=="colorIndex")||(token[i]=="normalIndex")) {
                const string & attribute=token[i++];
                string value;
                while(token[++i][0]!=']') value+=" "+token[i];
                currSt->attr(attribute,value);
            }
            else if(token[i]=="creaseAngle") {
                currSt->attr(token[i],token[i+1]);
                i++;
            }
        }
        else if(currSt->tag()=="ElevationGrid") {
            if((token[i]=="height")) {
                const string & attribute=token[i++];
                string value;
                while(token[++i][0]!=']') value+=" "+token[i];
                currSt->attr(attribute,value);
            }
            else if((token[i]=="xDimension")||(token[i]=="xSpacing")||(token[i]=="zDimension")||(token[i]=="zSpacing")) {
                currSt->attr(token[i],token[i+1]);
                i++;
            }
        }
        else if(currSt->tag()=="TextureTransform") {
            if(token[i]=="rotation") {
                currSt->attr(token[i],token[i+1]);
                i++;
            }
            else if((token[i]=="scale")||(token[i]=="center")||(token[i]=="translation")) {
                currSt->attr(token[i],token[i+1]+' '+token[i+2]);
                i+=2;
            }
        }
		else if((currSt->tag()=="LOD")) {
            if(token[i]=="range") {
                while((token[i][0]!='[')&&(i<token.size())) i++;
                string s;
                while((token[++i][0]!=']')&&(i<token.size())) s+=token[i]+" ";
                currSt->attr("range",trim(s));
            }
            else if(token[i]=="center") {
                currSt->attr(token[i],token[i+1]+' '+token[i+2]+' '+token[i+3]);
                i+=3;
            }
		}
        else if((token[i][0]=='[')||(token[i][0]==']')
                ||(token[i]=="point")||(token[i]=="color")
                ||(token[i]=="vector")) { }  // skip
        else if((currSt->tag()=="TextureCoordinate")||(currSt->tag()=="Coordinate")
                ||(currSt->tag()=="Color")||(currSt->tag()=="Normal"))
            s+=" "+token[i];
        else if((token[i]=="diffuseColor")||(token[i]=="emissiveColor")||(token[i]=="specularColor")) {
            if(i+3<token.size()) {
                currSt->attr(token[i],token[i+1]+' '+token[i+2]+' '+token[i+3]);
                i+=3;
            }
        }
        else if((token[i]=="transparency")||(token[i]=="shininess")||(token[i]=="ambientIntensity")
                ||(token[i]=="repeatS")||(token[i]=="repeatT")||(token[i]=="USE")) {
            currSt->attr(token[i],token[i+1]);
            i++;
        }
        else if(token[i]=="url") {
            string url;
            if(token[i+1][0]=='[') {
                url=token[i+2].substr(1,token[i+2].size()-2); // strip quotation marks
                i+=3;
            }
            else {
                url=token[i+1].substr(1,token[i+1].size()-2); // strip quotation marks
                i++;
            }
            if(url.find("file://")<url.size()) url=url.substr(url.find("file://")+7);
            currSt->attr("url",url);
        }
        else if(((token[i]=="translation")||(token[i]=="scale")||(token[i]=="center"))&&((i+3)<token.size())) {
            currSt->attr(token[i],token[i+1]+" "+token[i+2]+" "+token[i+3]);
            i+=3;
        }
        else if(((token[i]=="rotation")||(token[i]=="scaleOrientation"))&&((i+4)<token.size())) {
            currSt->attr(token[i],token[i+1]+" "+token[i+2]+" "+token[i+3]+" "+token[i+4]);
            i+=4;
        }
        else continue;
    }
    return xs;
}

static void tokenize (std::string s, vector<string> & token) {
    s=replaceAll(s.substr(0,s.find('#')),"]"," ] ");   // strip comments and add a leading space before brackets which 3ds forgets
    s=replaceAll(s,"["," [ ");  // there are lots of lazy exporters that do not stick to the whitespace rule...
    s=replaceAll(s,"{"," { ");
    s=replaceAll(s,"}"," } ");
    split(s,token,", \t\n\015"); // tokenize, remove commata
}

Xml vrmlToX3d (const string & s) {
    vector<string> vToken;
    size_t pos=0;
    while(pos<s.size()) {
        size_t lineEnd=s.find('\n',pos);
        tokenize(s.substr(pos,lineEnd-pos),vToken);
        pos=lineEnd+1;
    }
    return vrmlToX3d(vToken);
}



proNode* ioWrl::load(const std::string & filename) {
    ifstream file(filename.c_str(), std::ios::in);
    if (!file.good()) {
        cerr << "ioWrl ERROR: error in file "<< filename << " or file not found.\n";
        return 0;
    }
    string line;

    std::getline(file,line); // interpret first line
    float vrmlVersion=-1;
    if(line.size()>7) vrmlVersion=s2f(line.substr(7));
    else {
        cerr << "ioWrl ERROR: file "<< filename << " has corrupt VRML header.\n";
        file.close();
        return 0;
    }
    if(vrmlVersion < 2.0f)
        cerr << "ioWrl WARNING: VRML version "<< vrmlVersion << " not officially supported.\n";
	
    vector<string> vToken;
    while(!file.eof()) {
		std::getline(file,line);
        tokenize(line,vToken);
    }
    file.close();

	return proNode::interpret(vrmlToX3d(vToken));
}
