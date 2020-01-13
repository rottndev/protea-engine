// version 1.3 2008-01-17 by gerald.franz@o2online.de

#include "proStr.h"

#include <cstdio>
#include <iostream>

using namespace std;

#ifdef _WIN32
#include <windows.h>
std::string ws2s(const std::wstring &wstr) {
	std::string str;
	LPSTR szText = new CHAR[wstr.size()+1];
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szText, (int)wstr.size()+1, 0, 0);
	str = szText;
	delete[] szText;
	return str;
}

std::wstring s2ws(const std::string &str) {
	std::wstring wstr;
	LPWSTR szTextW = new WCHAR[str.size()+1];
	MultiByteToWideChar(0, 0, str.c_str(), -1, szTextW, (int)str.size()+1);
	wstr = szTextW;
	delete[] szTextW;
	return wstr;
}
#endif // _WIN32


size_t split(const string & input, vector<string> & output, const string & separators) {
    size_t numberOfElements=0;
    size_t start, stop;
    start = input.find_first_not_of(separators);
    while (start < input.size()) {
        stop = input.find_first_of(separators, start);
        if (stop > input.size()) stop = input.size();
        output.push_back(input.substr(start, stop - start));
        ++numberOfElements;
        start = input.find_first_not_of(separators, stop+1);
    }
    return numberOfElements;
}

size_t qsplit(const string & input, vector<string> & output, const string & separators) {
    size_t numberOfElements=0;
    size_t start, stop;
    start = input.find_first_not_of(separators);
    while (start < input.size()) {
        if(input[start]=='"') {
            stop = input.find_first_of("\"", start+1);
            stop = input.find_first_of(separators, stop);
        }
        else stop = input.find_first_of(separators, start);
        if (stop > input.size()) stop = input.size();
        output.push_back(input.substr(start, stop - start));
        ++numberOfElements;
        start = input.find_first_not_of(separators, stop+1);
    }
    return numberOfElements;
}

string trim(const string & s, const string & pattern) {
    if(!s.size()) return s;
    size_t b = s.find_first_not_of(pattern);
    if(b > s.size()) return "";
    return string(s, b, s.find_last_not_of(pattern) - b + 1);
}

string replaceAll(string s, const string & search, const string & repl) {
    size_t found = s.find(search);
    while(found <s.size()) {
        s.replace(found, search.size(), repl);
        found = s.find(search,found+repl.size());
    }
    return s;
}

string replaceChars(const string & s, const string & pattern, char ch) {
    size_t start, stop;
    string ret;
    start = s.find_first_not_of(pattern);
    while (start < s.size()) {
        stop = s.find_first_of(pattern, start);
        if (stop > s.size()) stop = s.size();
        ret+=s.substr(start, stop - start)+ch;
        start = s.find_first_not_of(pattern, stop+1);
    }
    return ret.substr(0,ret.find_last_not_of(pattern)+1);
}

void operator-=(string & s, unsigned int n) {
    s.erase(s.size()-n,n);
}

string i2s(long i) {
    char buf[32];
    sprintf(buf,"%li",i);
    return string(buf);
}

string f2s(double f) {
    char buf[32];
    sprintf(buf,"%f",f);
    string ret(buf);
    return ret.substr(0,ret.find_last_not_of("0")+2);
}

string f2s(double f, unsigned short nDigits) {
    char buf[32];
    string fmt="%.0"+i2s(nDigits)+'f';
    sprintf(buf,fmt.c_str(),f);
    return string(buf);;
}

string b2s(bool b, bool asText) {
    if(asText) {
        if(b) return "true";
        else return "false";
    }
    if(b) return "1";
    return "0";
}

string c2s(char ch) {
    string s("x");
    s[0]=ch;
    return s;
}

int s2i(const string & s) {
    return atoi(s.c_str());
}

unsigned int s2ui(const string & s) {
    return (unsigned int)atoi(s.c_str());
}


float s2f(const string & s) {
    return static_cast<float>(atof(s.c_str()));
}

bool s2b(const string & s) {
    if(s=="true") return true;
    if(s=="TRUE") return true;
    if(atoi(s.c_str())>0) return true;
    return false;
}

size_t s2f(const string & s, vector<float> & vFloat, const string & separators) {
    vector<string> words;
    vFloat.reserve(split(s,words,separators));
    for(size_t i=0; i<words.size(); ++i)
        vFloat.push_back(static_cast<float>(atof(words[i].c_str())));
    return words.size();
}

size_t s2i(const string & s, vector<int> & vInt, const string & separators) {
    vector<string> words;
    vInt.reserve(split(s,words,separators));
    for(size_t i=0; i<words.size(); ++i)
        vInt.push_back(atoi(words[i].c_str()));
    return words.size();
}

string toUpper(const string & s) {
    string retStr(s);
    for(size_t i=0; i<s.size(); ++i)
        retStr[i]=static_cast<char>(toupper(retStr[i]));
    return retStr;
}

string toLower(const string & s) {
    string retStr(s);
    for(size_t i=0; i<s.size(); ++i)
        retStr[i]=static_cast<char>(tolower(retStr[i]));
    return retStr;
}

string repr(const string & s, bool parentheses) {
    string ret;
	ret.reserve(s.size());
	for(unsigned int i=0; i<s.size(); ++i) switch(s[i]) {
        case ' ': ret += ' '; parentheses=true; break;
		case '\r': ret+="\\r"; parentheses=true; break;
		case '\n': ret+="\\n"; parentheses=true; break;
		case '\t': ret+="\\t"; parentheses=true; break;
		case '"': ret+="\\\""; parentheses=true; break;
		case '\\': ret+="\\\\"; parentheses=true; break;
        case '\0' : ret+="\\0"; parentheses=true; break;
        case '\a' : ret+="\\a"; parentheses=true; break;
        case '\b' : ret+="\\b"; parentheses=true; break;
        case '\f' : ret+="\\f"; parentheses=true; break;
        case '\v' : ret+="\\v"; parentheses=true; break;
		default: ret += s[i];
	}
    //cout << "\n>>" << s << "->" << ret << endl;
    return parentheses ? '"'+ret+'"' : ret;
}

string derepr(const std::string & s) {
    if(!s.size()||((s[0]!='"')||(s[s.size()-1]!='"'))) 
        return s; // strings which are not quoted are not decoded.
    string ret;
	ret.reserve(s.size()-2);
	for(unsigned int i=1; i+1<s.size(); ++i) 
        if(s[i]!='\\') ret+=s[i];
        else if(i+2<s.size()) switch(s[++i]) {
            case 'n' : ret+='\n'; break;
            case 't' : ret+='\t'; break;
            case 'r' : ret+='\r'; break;
            case '"' : ret+='"'; break;
            case '\\' : ret+='\\'; break;
            case '0' : ret+='\0'; break;
            case 'a' : ret+='\a'; break;
            case 'b' : ret+='\b'; break;
            case 'f' : ret+='\f'; break;
            case 'v' : ret+='\v'; break;
        }
    return ret;
}

unsigned int hex2ui(const string & s) {
    unsigned int factor=1;
    unsigned int ret=0;
    for(unsigned int i=0; i<s.size(); ++i) {
        unsigned int value=0;
        if(s[s.size()-i-1]>='a')
            value=10+s[s.size()-i-1]-'a';
        else if(s[s.size()-i-1]>='A')
            value=10+s[s.size()-i-1]-'A';
        else value=s[s.size()-i-1]-'0';
        ret+=value*factor;
        factor*=16;
    }
    return ret;
}

size_t match(const string & s, size_t pos, char chOpen, char chClose) {
    if(pos>=s.size()) return s.size();
    while((s[pos]!=chOpen)&&(pos<s.size())) ++pos;
    unsigned int nOpen=1;
    while((++pos<s.size())&&nOpen)
        if(s[pos]==chOpen) ++nOpen;
        else if(s[pos]==chClose) --nOpen;
    return pos;
}

bool wildcardMatch(std::string s, const std::string & pattern) {
    if(!pattern.size()||(pattern=="*")) return true;
    s=toLower(s);
    vector<string> vPattern;
    split(toLower(pattern),vPattern,";");
    for(size_t j=0; j<vPattern.size(); ++j) {
        bool openStart=vPattern[j][0]=='*';
        bool openEnd=vPattern[j][vPattern[j].size()-1]=='*';
        vector<string> parts;
        split(vPattern[j],parts,"*");
        if(!parts.size()) continue;
    
        size_t currPos=0;
        size_t matchPos;
        for(size_t i = 0; i < parts.size(); ++i) {
            matchPos=s.substr(currPos).find(parts[i]);
            if(matchPos>s.size()) goto next; // part not included
            if(!i&&!openStart&&matchPos) goto next; // parts[0] has to be at the start
            currPos+=matchPos+parts[i].size();
            if((i==parts.size()-1)&&!openEnd&&currPos<s.size()) goto next;
        }
        return true;
        next: ;
    }
    return false;
}

int isNumber(const string & s) {
    size_t i= s.find_first_not_of(" \t\n\015");
    if(i>=s.size()) return 0;
    if((s[i]=='-')||(s[i]=='+'))
        if(++i==s.size()) return 0;
    int ret=1; // integer
    bool dotAllowed=true;
    bool eAllowed=true;
    if(s[i]=='.') {
        if(++i==s.size()) return 0;
        ret=2; // float
        dotAllowed=false;
    }
    while(i<s.size()) {
        if(isspace(s[i])) break;
        if(!isdigit(s[i])&&(tolower(s[i])!='e')&&(s[i]!='.')) 
            return 0;
        if(tolower(s[i])=='e') {
            if(eAllowed) {
                if(i+1==s.size()) return 0;
                ret=2;
                eAllowed=dotAllowed=false;
            }
            else return 0;
        }
        else if(s[i]=='.') {
            if(dotAllowed) {
                ret=2;
                dotAllowed=false;
            }
            else return 0;
        }
        ++i;
    }
    return ret;
}

char** tokenize(char* input) {
	size_t nTokenMax=2;
	size_t nToken=0;
	char** token =(char**)malloc(nTokenMax*sizeof(char*));
	token[nToken]=0;
	unsigned int currP=0;
	bool wordFound=false;
	char* p0 = input;
	while(*p0) {
		if((*p0=='\n')||(*p0=='\r')) {
			*p0=0;
			continue;
		}
		if((*p0==' ')||(*p0=='\t')) {
			if(wordFound) {
				wordFound=false;
				*p0=0;
				++currP;
			}
		}
		else if(!wordFound) {
			wordFound=true;
			token[nToken++]=p0;
			if(nToken==nTokenMax) {
				nTokenMax*=2;
				token = (char**)realloc(token, nTokenMax*sizeof(char*));
			}
			token[nToken]=0;
		}
		++p0;
	}
	return token;
}
