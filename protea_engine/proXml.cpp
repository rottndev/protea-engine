#include "proXml.h"
#include <cstdio>
#include <cctype>

using namespace std;

vector<pair<string,string> > Xml::sv_code;

//--- class XmlTokenizer -------------------------------------------

class XmlTokenizer {
public:
    /// constructor form a string
    XmlTokenizer(const std::string & str) : s(str), i(0), tagMode(false) { }
    /// returns next token or an empty string in case no further token is available
    std::string next();
protected:
    /// stores string to be tokenized
    std::string s;
    /// stores current read position
    size_t i;
    /// stores current read context
    bool tagMode;
    /// stores next token, if already determined
    std::string m_nextToken;
};

string XmlTokenizer::next() {
	const unsigned char ESC=27;
	string word;
	int quotMode=0;
	bool tokenComplete = false;   
	while(m_nextToken.size()||(i<s.size())) {
		
		if(m_nextToken.size()) {
			word=m_nextToken; 
			m_nextToken.clear();
			return word;
		}

		switch(s[i]) {
		case '"': 
		case '\'': 
			if(tagMode) {
				if(!quotMode) quotMode=s[i];
				else if(quotMode==s[i]) quotMode=0;
			}
			word+=s[i]; 
			break;
		case '<': 
			if(!quotMode&&(i+4<s.size())&&(s.substr(i,4)=="<!--")) // strip comments
				i=s.find("-->",i+4)+2;
			else if(!quotMode&&(i+9<s.size())&&(s.substr(i,9)=="<![CDATA[")) { // interpet CDATA
				size_t b=i+9;
				i=s.find("]]>",b)+3;
				if(!word.size()) return s.substr(b,i-b-3);
				tokenComplete = true;
				m_nextToken=s.substr(b,i-b-3);
				--i;
			}
			else if(!quotMode&&(i+1<s.size())&&((s[i+1]=='?')||(s[i+1]=='!'))) // strip meta information
				i=s.find('>',i+2);
			else if(!quotMode&&!tagMode) {
				if((i+1<s.size())&&(s[i+1]=='/')) {
					m_nextToken=ESC;
					i=s.find('>',i+2);
				}
				else {
					m_nextToken='<';
					tagMode=true; 
				}
				tokenComplete = true;
			}
			else word+=s[i];
			break;
		case '/': 
			if(tagMode&&!quotMode) {
				tokenComplete = true;
				if(i+1<s.size()&&(s[i+1]=='>')) {
					tagMode=false;
					m_nextToken=ESC;
					++i;
				}
				else word+=s[i];
			}                    
			else word+=s[i];
			break;
		case '>': 
			if(!quotMode&&tagMode) {
				tagMode=false;
				tokenComplete = true;
				m_nextToken='>'; 
			}                    
			else word+=s[i];
			break;
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			if(tagMode&&!quotMode) {
				if(word.size()) tokenComplete=true;
			}
			else word+=s[i];
			break;
		default: word+=s[i];
		}
        if(i<s.size()) ++i;
        if((i>=s.size())||(tokenComplete&&word.size())) {
            tokenComplete=false;
            size_t b = word.find_first_not_of(" \t\n\015"); // trim whitespace
            if(b < word.size()) {
                word=word.substr(b, word.find_last_not_of(" \t\n\015") - b + 1); 
                if(word.size()) break;
            }
            else word.clear();
        }
    }
    return word;
}

//--- string utilities ---------------------------------------------

static string replaceAll(string s, const string & search, const string & repl) {
    size_t found = s.find(search);
    while(found <s.size()) {
        s.replace(found, search.size(), repl);
        found = s.find(search,found+repl.size());
    }
    return s;
}

static string char2code(unsigned char ch) {
	string s("&#"), c("0");
	if(ch>99) {
		c[0]=ch/100+48;
		s+=c;
	}
	if(ch>9) {
		c[0]=(ch%100)/10+48;
		s+=c;
	}
	c[0]=ch%10+48;
	return s+c+';';
}

void Xml::registerCode(const std::string & decoded, const std::string & encoded) {
    for(vector<pair<string,string> >::iterator it=sv_code.begin(); it!=sv_code.end(); ++it)
        if(it->first==decoded) return;
    sv_code.push_back(make_pair(decoded, encoded));
}

void Xml::registerDefaultCodes() {
    sv_code.push_back(make_pair("&", "&amp;"));
    sv_code.push_back(make_pair("<", "&lt;"));
    sv_code.push_back(make_pair(">", "&gt;"));
    sv_code.push_back(make_pair("\"","&quot;"));
    sv_code.push_back(make_pair("'", "&apos;"));
}

string Xml::encode(string s) {
    for(vector<pair<string,string> >::iterator it=sv_code.begin(); it!=sv_code.end(); ++it)
        s = replaceAll(s, it->first, it->second);
	for(size_t pos=0; pos<s.size(); ++pos)
		if(s[pos]<0) s.replace(pos,1,char2code(static_cast<unsigned char>(s[pos])));
    return s;
}

string Xml::decode(string s) {
    size_t found = s.find("&#");
    while((found+5 <s.size())&&(s[found+5]==';')&&isdigit(s[found+2])&&isdigit(s[found+3])&&isdigit(s[found+4]) ) {
		char ch[]=" ";
		ch[0]=100*(s[found+2]-48)+10*(s[found+3]-48)+(s[found+4]-48);
        s.replace(found, 6, ch);
        found = s.find("&#",found+1);
    }
    for(size_t i=sv_code.size()-1;i<sv_code.size(); --i)
        s = replaceAll(s, sv_code[i].second, sv_code[i].first);
    return s;
}

//--- Creation / Deletion methods ----------------------------------

Xml::Xml(const Xml &source) : m_tag(source.m_tag), mv_attr(source.mv_attr), mp_parent(0) {
    for (size_t i=0; i<source.mv_elem.size(); ++i)
        if(source.mv_elem[i].first) {
            mv_elem.push_back(make_pair(new Xml(*source.mv_elem[i].first),string()));
            mv_elem.back().first->mp_parent=this;
        }
        else mv_elem.push_back(make_pair(static_cast<Xml*>(0),source.mv_elem[i].second));
}

Xml::~Xml() { // destructor
    for(vector< pair<Xml*,string> >::iterator iter=mv_elem.begin(); iter<mv_elem.end(); ++iter) if(iter->first) {
        iter->first->mp_parent=0; // otherwise somehow recursive...
        delete iter->first;
    }
    if(mp_parent) mp_parent->erase(this);
}

Xml & Xml::operator=(const Xml & source) {
    if (this==&source) return *this; // first test for self reference
    clear();                         // then cleanup memory
    m_tag=source.m_tag;            // finally copy source
    mv_attr=source.mv_attr;
    for (size_t i=0; i<source.mv_elem.size(); ++i)
        if(source.mv_elem[i].first) {
            mv_elem.push_back(make_pair(new Xml(*source.mv_elem[i].first),string()));
            mv_elem.back().first->mp_parent=this;
        }
        else mv_elem.push_back(make_pair(static_cast<Xml*>(0),source.mv_elem[i].second));
    return *this;
}

bool Xml::operator==(const Xml & xs) const {
	if(m_tag!=xs.m_tag) return false;
	if(mv_elem.size()!=xs.mv_elem.size()) return false;
	if(mv_attr.size()!=xs.mv_attr.size()) return false;
	for(size_t i=0; i<mv_attr.size(); ++i)
		if(mv_attr[i]!=xs.mv_attr[i]) return false; // not clean because attribute sequence is not meaningful!
	for(size_t i=0; i<mv_elem.size(); ++i) {
		if(mv_elem[i].first) {
			if(!xs.mv_elem[i].first) return false;
			if(*mv_elem[i].first!=*xs.mv_elem[i].first) return false;
		}
		else if(mv_elem[i].second!=xs.mv_elem[i].second)
			return false;
	}
	return true;
}

void Xml::clear() {
    m_tag="tag";
    mv_attr.clear();
    for(vector< pair<Xml*,string> >::iterator iter=mv_elem.begin(); iter<mv_elem.end(); ++iter) if(iter->first) {
        iter->first->mp_parent=0; // otherwise somehow recursive...
        delete iter->first;
    }
    mv_elem.clear();
    if(mp_parent) {
        for(vector< pair<Xml*,string> >::iterator iter=mp_parent->mv_elem.begin(); iter<mp_parent->mv_elem.end(); ++iter) 
            if((iter->first)&& (iter->first==this)) {
                mp_parent->mv_elem.erase(iter);
                break;
            }
    }
}

//--- changing content, attributes and links to other statements ---

Xml & Xml::append(const Xml & xmlSt) {
	Xml *pXmlSt=new Xml(xmlSt);
	pXmlSt->mp_parent=this;
	mv_elem.push_back(make_pair(pXmlSt,string()));
	return *pXmlSt;
}

void Xml::append(const string & s) { 
	if(mv_elem.size()&&!mv_elem.back().first)
		mv_elem.back().second+=" "+s;
	else mv_elem.push_back(std::make_pair(static_cast<Xml*>(0),s)); 	
}

int Xml::erase(Xml * xmlSt) {
    for(vector<pair<Xml*,string> >::iterator iter=mv_elem.begin(); iter<mv_elem.end(); iter++)
        if((iter->first)&& (iter->first==xmlSt)) {
            xmlSt->mp_parent=0;
            mv_elem.erase(iter);
            delete xmlSt;
            return 0;
        }
    return 1;
}

string Xml::attr(const string & name) const {
	for(size_t i=0; i<mv_attr.size(); i+=2)
		if(mv_attr[i]==name) return mv_attr[i+1];
	return string();
}

void Xml::attr(const string & name, const string & value) {
	for(vector<string>::iterator iter=mv_attr.begin(); iter<mv_attr.end(); iter+=2) if(*iter==name) {
		if(value.empty()) { // erase attribute
			iter=mv_attr.erase(iter);
			if(iter!=mv_attr.end()) mv_attr.erase(iter);
		}
		else *(iter+1)=value;
		return;
	}
	if(value.empty()) return;
	mv_attr.push_back(name);
	mv_attr.push_back(value);
}

Xml * Xml::child(const string & tagName, const string & attrKey, const string & attrValue) {
	for(vector< pair<Xml*,string> >::iterator iter=mv_elem.begin(); iter<mv_elem.end(); iter++)
		if(iter->first && ((iter->first->m_tag==tagName)||!tagName.size()) )
			if(!attrKey.size()||!attrValue.size()||(iter->first->attr(attrKey)==attrValue))
				return iter->first;
	return 0;
}

const Xml * Xml::child(const string & tagName, const string & attrKey, const string & attrValue) const {
	for(vector< pair<Xml*,string> >::const_iterator iter=mv_elem.begin(); iter<mv_elem.end(); iter++)
		if(iter->first && ((iter->first->m_tag==tagName)||!tagName.size()) ) 
			if(!attrKey.size()||!attrValue.size()||(iter->first->attr(attrKey)==attrValue))
				return iter->first;
	return 0;
}

Xml * Xml::find(const string & tagName, const string & attrKey, const string & attrValue) {
    if((m_tag==tagName)||!tagName.size())
        if(!attrKey.size()||!attrValue.size()||(attr(attrKey)==attrValue))
            return this;
    for(vector< pair<Xml*,string> >::iterator iter=mv_elem.begin(); iter<mv_elem.end(); iter++) if(iter->first) {
        Xml *ret=iter->first->find(tagName,attrKey,attrValue);
        if(ret) return ret;
    }
    return 0;
}

const Xml * Xml::find(const string & tagName, const string & attrKey, const string & attrValue) const {
    if((m_tag==tagName)||!tagName.size())
        if(!attrKey.size()||!attrValue.size()||(attr(attrKey)==attrValue))
            return this;
    for(vector< pair<Xml*,string> >::const_iterator iter=mv_elem.begin(); iter<mv_elem.end(); iter++) if(iter->first) {
        Xml *ret=iter->first->find(tagName,attrKey,attrValue);
        if(ret) return ret;
    }
    return 0;
}

//--- file input output --------------------------------------------

Xml Xml::load(const string & filename) {
    FILE * file=fopen(filename.c_str(),"r");
    if(!file) {
        fprintf(stderr,"Xml ERROR: \"%s\" file error or file not found!",filename.c_str());
        return Xml("ERROR");
    }
    fseek (file , 0 , SEEK_END);
    size_t sz = ftell (file);
    rewind (file);
    char* buffer = new char[sz];
    fread (buffer,1,sz,file);
    fclose(file);
    buffer[sz-1]=0;
    Xml xml;
    xml.eval(buffer);
    delete[] buffer;
    return xml;
}

int Xml::save(const string & filename) const {
    FILE * file=fopen(filename.c_str(),"w");
    if (!file) {
        fprintf(stderr,"Xml ERROR: \"%s\" write file error!",filename.c_str());
        return 1;
    }
    string header("<?xml version=\"1.0\"?>\n<!-- generated by protea::Xml -->\n\n");
    fwrite(header.c_str(),sizeof(header[0]),header.size(),file);
    string body(str());
    fwrite(body.c_str(),sizeof(body[0]),body.size(),file);
    fclose(file);
    return 0;
}

void Xml::eval(const string & s) {
	const unsigned char ESC=27;
	clear();
	m_tag.clear();
	Xml *currSt = 0;
	XmlTokenizer tokenizer(s);
	string token;
	while((token = tokenizer.next()).size()) {
		if(token[0]=='<') { // new tag found
			if(currSt) {
				currSt->append(Xml(tokenizer.next()));
				currSt=currSt->mv_elem.back().first;
			}
			else {
				if(!m_tag.size()) m_tag=tokenizer.next();
				else return;
				currSt=this;
			}
			while((token = tokenizer.next()).size()&&(token[0]!='>')&&(token[0]!=ESC)) { // parse tag header
				size_t sepPos=token.find('=');
				if(sepPos<token.size()) { // regular attribute
					string aName(token,0,sepPos);
					string aValue(token, sepPos+2, token.rfind(token[sepPos+1])-sepPos-2);
					currSt->attr(aName, decode(aValue));
				}
			}
			if((token[0]==ESC)&&currSt)
				currSt=currSt->parent(); // this tag has no content, only attributes
		}
		else if((token[0]==ESC)&&currSt) currSt=currSt->parent(); // tag is over
		else if(currSt) currSt->append(decode(token));
	}
}

string Xml::str(unsigned int nTabs) const {
    string s;
    unsigned int i;
    string tabStr;
    for(i=0; i<nTabs; ++i) tabStr+='\t';
    s+=tabStr+"<"+m_tag;
    for(i=0; i<mv_attr.size(); i+=2)
        s+= " "+mv_attr[i]+"=\""+encode(mv_attr[i+1])+"\"";
    if(!mv_elem.size()) s+="/>\n";
    else {
        s+=">";
        if((mv_elem.size()==1)&&!mv_elem[0].first) // content only
            s+=' '+mv_elem[0].second+" </"+m_tag+">\n";
        else { // substatements only
            s+='\n';
            for(i=0; i<mv_elem.size();i++)
                s+=mv_elem[i].first ? mv_elem[i].first->str(nTabs+1) : (mv_elem[i].second+'\n');
            s+=tabStr+"</"+m_tag+">\n";
        }
    }
    return s;
}
