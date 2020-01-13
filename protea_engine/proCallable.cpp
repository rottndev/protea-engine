#include "proCallable.h"
#include "proStr.h"
#include "proIo.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
};
#include <cstdio>
using namespace std;

//--- class Callable -----------------------------------------------

std::map<std::string,Callable*> Callable::sm_instance;

Callable::~Callable() { 
	if(m_name.size()) {
		map<string,Callable*>::iterator it = sm_instance.find(m_name);
		if(it!=sm_instance.end()) sm_instance.erase(it);
	}
}

const Callable & Callable::operator=(const Callable & source) {
	if(&source!=this) {
		if(m_name.size()) {
			map<string,Callable*>::iterator it = sm_instance.find(m_name);
			if(it!=sm_instance.end()) sm_instance.erase(it);
		}
		m_name = source.m_name;
		if(m_name.size())
			sm_instance[m_name]=this;
	}
	return *this;
}

void Callable::name(const std::string & s) { 
	if(m_name.size()) {
		map<string,Callable*>::iterator it = sm_instance.find(m_name);
		if(it!=sm_instance.end()) sm_instance.erase(it);
	}
	m_name = s; 
	sm_instance[s]=this; 
}

Var Callable::info() const { return Var::null; }

Callable* Callable::instance(const std::string & name) {
	map<string,Callable*>::iterator it = sm_instance.find(name);
	return it==sm_instance.end() ? 0 : it->second;
}

//--- class Var ----------------------------------------------------
// instantiation of static object
Var Var::null;

static const char TABLE_OPEN = '{';
static const char TABLE_CLOSE = '}';
static const char KEY_VALUE_SEPARATOR = '=';

Var::Var(const std::string & s) : m_type(STRING) { 
    u.s=new std::string(derepr(s));
}

Var::Var(const char * s) : m_type(STRING) { 
    u.s=new std::string(derepr(s)); 
}

Var::~Var() {
	switch(m_type) {
	default:          return;
	case STRING: delete u.s; return;
	case ARRAY: delete u.v; return;
	case MAP:    delete u.m; return;
	}
}

Var::Var(const Var & source) : m_type(source.m_type) {
	if(m_type==STRING)
		u.s=new std::string(*source.u.s);
	else if(m_type==ARRAY)
		u.v=new vector<Var>(*source.u.v);
	else if(m_type==MAP)
		u.m=new map<std::string,Var>(*source.u.m);
	else u=source.u;
}

const Var & Var::operator=(const Var & source) {
	if(&source==this) return *this;
	if(m_type==STRING) delete u.s;
	else if(m_type==ARRAY) delete u.v;
	else if(m_type==MAP) delete u.m;
	m_type=source.m_type;
	if(m_type==STRING)
		u.s=new std::string(*source.u.s);
	else if(m_type==ARRAY)
		u.v=new vector<Var>(*source.u.v);
	else if(m_type==MAP)
		u.m=new map<std::string,Var>(*source.u.m);
	else u=source.u;
	return *this;
}

bool Var::operator==(const Var & v) const {
	if(&v==this) return true;
	if(m_type!=v.m_type) return false;
	if(m_type==ARRAY) {
        if(u.v->size()!=v.u.v->size()) return false;
        for(unsigned int i=0; i<u.v->size(); ++i)
            if((*u.v)[i]!=(*v.u.v)[i]) return false;
    }
	else if(m_type==MAP) {
        map<std::string,Var>::iterator it=u.m->begin();
        map<std::string,Var>::iterator jt=v.u.m->begin();
        for(; (it!=u.m->end()) && (jt!=v.u.m->end()) ; ++it, ++jt) {
            if(it->first!=jt->first) return false;
            if(it->second!=jt->second) return false;
        }
        if(it!=u.m->end()) return false;
        if(jt!=v.u.m->end()) return false;
    }
	else if(m_type==STRING)
		return (*u.s)==(*v.u.s);
	else if(m_type==INT)
		return u.i==v.u.i;
	else if(m_type==FLOAT)
		return u.f==v.u.f;
	return true;
}

int Var::integer() const {
	switch(m_type) {
	default:          return 0;
	case INT:    return u.i;
	case FLOAT:  return static_cast<int>(u.f);
	case STRING: return s2i(*(u.s));
	}
}

float Var::number() const {
	switch(m_type) {
	default:     return 0.0f;
	case INT:    return static_cast<float>(u.i);
	case FLOAT:  return u.f;
	case STRING: return s2f(*(u.s));
	}
}

string Var::string() const {
	switch(m_type) {
	default:     return this->repr();
	case NONE:   return "";
	case INT:    return i2s(u.i);
	case FLOAT:  return f2s(u.f);
	case STRING: return *(u.s);
	}
}

bool Var::boolean() const {
	switch(m_type) {
	default:          return false;
	case INT:    return u.i!=0;
	case FLOAT:  return u.f!=0.0f;
	case STRING: return toLower(*(u.s))=="true";
	}
}

size_t Var::numArray(float* arr, size_t size) const {
	switch(m_type) {
	default:          return 0;
	case INT:    arr[0]=(float)u.i; return 1;
	case FLOAT:  arr[0]=u.f; return 1;
	case ARRAY: {
		size_t i;
		for(i=0; (i<u.v->size())&&(i<size)&&((*u.v)[i].type()&Var::NUM); ++i)
			arr[i]=(*u.v)[i].number();
		return i;
	}
	}
}

string Var::repr() const {
	switch(m_type) {
	default:     return "";
	case INT:    return i2s(u.i);
	case FLOAT:  return f2s(u.f);
	case STRING: return ::repr(*(u.s),true);
	case ARRAY: {
		std::string s("{ ");
		s[0]=TABLE_OPEN;
		for(unsigned int i=0; i<u.v->size(); ++i) {
			if(i) s+=", ";
			s+=(*u.v)[i].repr();
		}
		s+=" }";
		s[s.size()-1]=TABLE_CLOSE;
		return s;
	}
	case MAP: {
		std::string s("{ ");
		s[0]=TABLE_OPEN;
		for(map<std::string,Var>::iterator it = u.m->begin(); it!=u.m->end(); ++it) {
			if(it!=u.m->begin()) s+=", ";
			s+=::repr(it->first)+KEY_VALUE_SEPARATOR+it->second.repr();
		}
		s+=" }";
		s[s.size()-1]=TABLE_CLOSE;
		return s;
	}
	}
}

Var Var::interpret(std::string s) {
    s=trim(s);
    if(!s.size()) // empty string, NONE
        return Var::null;
	
    int numType=isNumber(s);
    if(numType==1) // integer
        return Var(s2i(s));
    if(numType==2) // float
        return Var(s2f(s));
	
    if((s[0]==TABLE_OPEN)&&(s[s.size()-1]==TABLE_CLOSE)) { // array or map
        Var ret;
        vector<std::string> vWord;
		while(s.find('#')<s.size()) // not completely clean (does not consider quotes)
			s.erase(s.find('#'),s.find('\n',s.find('#'))-s.find('#'));
        s=trim(s.substr(1,s.size()-2));
        
        size_t pos=0;
        while(pos<s.size()) {
            size_t end=pos;
            switch(s[pos]) {
            case TABLE_OPEN:
                end=match(s,end,TABLE_OPEN, TABLE_CLOSE);
				break;
            case '"':
                if(pos==s.size()-1) break; // isolated end "
                end=pos+1;
                do end=static_cast<unsigned int>(s.find('"',end+1)); while(s[end-1]=='\\');
                ++end;
				break;
            default:
	            end=static_cast<unsigned int>(s.find_first_of(",= \t\n\r",end));
				break;
            }
            unsigned int sep=static_cast<unsigned int>(s.find_first_of(",=",end));
            
			if((end>=s.size())||(s[end]==',')) { // array element
				Var tmp(Var::interpret(s.substr(pos,end-pos)));
	            if(tmp.m_type!=NONE) ret.append(tmp);
				pos=(sep>=s.size())?sep:static_cast<unsigned int>(s.find_first_not_of(" \t\n\r",sep+1));
			}
            else if(s[end]==KEY_VALUE_SEPARATOR) { // hash element
				std::string key=s.substr(pos,end-pos);
				if((key[0]=='"')&&(key[key.size()-1]=='"'))
					key=key.substr(1,key.size()-2);
                // FIXME: add Variable/command substution
                
                end=pos=static_cast<unsigned int>(s.find_first_not_of(" \t\n\r",sep+1));
                switch(s[pos]) {
				case TABLE_OPEN:
					end=match(s,end,TABLE_OPEN, TABLE_CLOSE);
					break;
                case '"':
                    if(pos==s.size()-1) break; // isolated end "
                    end=pos;
                    do end=static_cast<unsigned int>(s.find('"',end+1)); while(s[end-1]=='\\');
                    ++end;
					break;
                default:
                    end=static_cast<unsigned int>(s.find_first_of(", \t\n\r",end));
                }
				Var tmp(Var::interpret(s.substr(pos,end-pos)));
                ret.set(key,tmp);
				end=static_cast<unsigned int>(s.find(',',end));
	            pos=(end>=s.size())?end:static_cast<unsigned int>(s.find_first_not_of(" \t\n\r",end+1));
            }
			else {
				fprintf(stderr, "Var::interpret() ERROR: unexpected character '%c' encountered.\n", s[end]);
				return ret;
			}
        }
        return ret;
    }
    // string:
    if(((s[0]=='"')&&(s[s.size()-1]=='"'))||((s[0]=='\'')&&(s[s.size()-1]=='\'')))
        return Var(derepr(s));
	else return Var(s); // FIXME: add Variable/command substution
}

unsigned int Var::size() const {
	switch(m_type) {
	default: return 0;
	case STRING: return static_cast<unsigned int>(u.s->size());
	case ARRAY: return static_cast<unsigned int>(u.v->size());
	}
}

/// array element access operator
Var & Var::operator[](int n) {
    //clog << "Var::operator[" << n << "]" << endl;
	if(m_type!=ARRAY) {
        if(n==0) return *this;
        else return Var::null;
    }
	if(static_cast<unsigned int>(abs(n))>=u.v->size()) return Var::null;
	return (*u.v)[ n<0 ? u.v->size()+n : n ];
}

/// array element access operator, const
const Var & Var::operator[](int n) const {
    //clog << "Var::operator[" << n << "]const" << endl;
	if(m_type!=ARRAY) {
        if(n==0) return *this;
        else return Var::null;
    }
	if(static_cast<unsigned int>(abs(n))>=u.v->size()) {
        return Var::null;
    }
	return (*u.v)[ n<0 ? u.v->size()+n : n ];
}

/// hash element access operator
Var & Var::operator[](std::string key) {
    //clog << "Var::operator[" << key << "]" << endl;
    if(!key.size()) return *this;
	if((m_type!=MAP)&&(m_type!=ARRAY)) return Var::null;
    // resolve concatenated keys:
    unsigned int firstPoint=static_cast<unsigned int>(key.find("."));
    std::string subkey;
    if(firstPoint<key.size()) {
        subkey=key.substr(firstPoint+1);
        key=key.substr(0,firstPoint);
    }
    // fetch value:
    if((isNumber(key)==1)&&(m_type==ARRAY)) {
        int n=s2i(key);
        if(static_cast<unsigned int>(abs(n))>u.v->size()) return Var::null;
        return subkey.size() ? (*u.v)[ n<0 ? u.v->size()+n : n ][subkey] : (*u.v)[ n<0 ? u.v->size()+n : n ];
    }
    if((m_type!=MAP)) return Var::null;
    map<std::string,Var>::iterator it=u.m->find(key);
    return it==u.m->end() ? Var::null : subkey.size() ? it->second[subkey] : it->second;
}

/// hash element access operator, const
const Var & Var::operator[](std::string key) const {
    //clog << "Var::operator[" << key << "] const" << endl;    
    if(!key.size()) return *this;
	if((m_type!=MAP)&&(m_type!=ARRAY)) return Var::null;
    // resolve concatenated keys:
    unsigned int firstPoint=static_cast<unsigned int>(key.find("."));
    std::string subkey;
    if(firstPoint<key.size()) {
        subkey=key.substr(firstPoint+1);
        key=key.substr(0,firstPoint);
    }
    // fetch value:
    if((isNumber(key)==1)&&(m_type==ARRAY)) {
        int n=s2i(key);
        if(static_cast<unsigned int>(abs(n))>u.v->size()) return Var::null;
        return subkey.size() ? (*u.v)[ n<0 ? u.v->size()+n : n ][subkey] : (*u.v)[ n<0 ? u.v->size()+n : n ];
    }
    if((m_type!=MAP)) return Var::null;
    map<std::string,Var>::iterator it=u.m->find(key);
    return it==u.m->end() ? Var::null : subkey.size() ? it->second[subkey] : it->second;
}

Var & Var::append(const Var & v) {
	if(m_type==NONE) {
		m_type=ARRAY;
		u.v=new vector<Var>;
	}
	if(m_type==ARRAY) u.v->push_back(v);
	return *this;
}

Var & Var::set(const std::string & key, const Var & v) {
    if(!key.size()) {
        *this=v;
        return *this;
    }
	if(m_type==NONE) {
		m_type=MAP;
		u.m=new map<std::string,Var>;
	}
	if(m_type!=MAP) return *this;
    unsigned int firstPoint=static_cast<unsigned int>(key.find("."));
    std::string subkey, mainkey;
    if(firstPoint<key.size()) {
        subkey=key.substr(firstPoint+1);
        mainkey=key.substr(0,firstPoint);
    }
    else mainkey=key;
    Var & search=operator[](mainkey);
    if(!subkey.size()) {
        if(&search!=&Var::null) search=v;
        else u.m->insert(make_pair(mainkey,v));
    }
    else if(&search!=&Var::null) search.set(subkey,v);
    else {
        Var vnew;
        vnew.set(subkey,v);
        u.m->insert(make_pair(mainkey,vnew));
    }
	return *this;
}

Var Var::info() const {
	if(m_type!=MAP) return Var::null;
	Var ret;
	for(map<std::string,Var>::iterator it = u.m->begin(); it!=u.m->end(); ++it) 
		ret.append(it->first);
	return ret;
}


int Var::erase(int n) {
	if(m_type!=ARRAY) return 1;
	if(static_cast<unsigned int>(abs(n))>u.v->size()) return 1;
	if(n<0) n+=static_cast<unsigned int>(u.v->size());
	u.v->erase(u.v->begin()+n);
	return 0;
}

int Var::erase(const std::string & key) {
	if(m_type!=MAP) return 1;
    for(map<std::string,Var>::iterator it=u.m->begin(); it!=u.m->end(); ++it)
        if(it->first==key) { u.m->erase(it); return 0; }
	return 1;
}

void Var::clear() {
	if(m_type==STRING) delete u.s;
	else if(m_type==ARRAY) delete u.v;
	else if(m_type==MAP) delete u.m;
	m_type=NONE;
    u.i=0; 
}

ostream & operator<<(std::ostream & os, const Var & v) {
	return ( os << v.string() ); 
}

//--- VMCallable auxiliary functions -------------------------------

static const char * readFileChunk(lua_State *L, void *ud, size_t *size) {
    FILE *f=(FILE *)ud;
    static char buff[512];
    if (feof(f)) return NULL;
    *size=fread(buff,1,sizeof(buff),f);
    return (*size>0) ? buff : NULL;
}

/// pushes a global or nested Lua variable onto the stack
static int push(lua_State* L, const char* name) {
	// scan for name parts:
	const char SEP='.';
	size_t begin=0, end=0;
	size_t nameLen = strlen(name);
	if(!nameLen) return 0;
	int counter =1;
	while((end<nameLen)&&(name[end]!=SEP)) ++end;

	lua_pushlstring(L, name, end);
	lua_gettable(L, lua_istable(L, -2) ? -2 : LUA_GLOBALSINDEX);
	while((name[end]==SEP)&&lua_istable(L, -1)) { // nested variable
		++counter;
		begin=++end;
		while((end<nameLen)&&(name[end]!=SEP)) ++end;
		if(end==begin) break;
		
		lua_pushlstring(L, &name[begin], end-begin);
		lua_gettable(L, -2);
	}
	if(lua_isnil(L,-1)) { // operation failed
		lua_pop(L, counter);
		return 0;
	}
	return counter;
}

/// converts a lua variable on the stack to a protea Var object
static Var lua2var(lua_State *L, int index) {
	switch(lua_type(L, index)) {
		case LUA_TBOOLEAN: return lua_toboolean(L, index);
		case LUA_TNUMBER: {
			double f = lua_tonumber(L, index);
			return (f==(int)f) ? (int)f : f;
		}
		case LUA_TSTRING: return lua_tostring(L, index);
		case LUA_TTABLE: {
			// convention: if there are numeric indices, treat table as array, otherwise as key/value map
			Var v;
			int i=1;
			lua_rawgeti(L, index, 1);
			if(!lua_isnil(L, -1)) while(!lua_isnil(L, -1)) {
				v.append(lua2var(L,-1));
				lua_pop(L, 1);
				lua_rawgeti(L, index, ++i);
			}
			else while (lua_next(L, index<0 ? index-1 : index)) {
				if(lua_type(L, -2)==LUA_TSTRING) 
					v.set(lua_tostring(L,-2), lua2var(L,-1));
				lua_pop(L, 1);
			}
			if(lua_isnil(L, -1)) lua_pop(L, 1);
			return v;
		}
		default:
			return Var::null;
	}
}

/// pushes a Var onto the Lua stack
/** \return number of values pushed onto the Lua stack, normally 1 */
static int var2lua(lua_State *L, const Var & v) {
	switch(v.type()) {
	default:
		return 0;
	case Var::INT:    
		lua_pushinteger(L, v.integer());
		return 1;
	case Var::FLOAT:
		lua_pushnumber(L, v.number());
		return 1;
	case Var::STRING: 
		lua_pushstring(L, v.string().c_str());
		return 1;
	case Var::ARRAY:
		lua_createtable(L, v.size(), 0);
		for(unsigned int i=0; i<v.size(); ++i) {
			var2lua(L, v[i]);
			lua_rawseti(L,-2,i+1);
		}
		return 1;
	case Var::MAP: 
		Var keys = v.info();
		lua_createtable(L, 0, keys.size());
		for(unsigned int i=0; i<keys.size(); ++i) {
			lua_pushstring(L, keys[i].string().c_str());
			var2lua(L, v[keys[i].string()]);
			lua_rawset(L,-3);
		}
		return 1;
	}
}

//--- class VMCallable ---------------------------------------------

map<lua_State*,VMCallable*> VMCallable::sm_vm;

VMCallable::VMCallable(bool openDefaultLibs) : m_status(0) {
	L=luaL_newstate();
	if(openDefaultLibs) luaL_openlibs(L);
	sm_vm[L]=this;
}

VMCallable::~VMCallable() {
	sm_vm.erase(sm_vm.find(L));
	lua_close(L);
}

int VMCallable::load(const std::string & filename) {
    FILE * file=fopen(filename.c_str(), "rb");
    if(!file) {
        m_error="ERROR: cannot open script file ["+filename+"]";
        return 1;
    }
    int ret=0;
    if (lua_load(L,readFileChunk, file, filename.c_str())) {
        m_error="Syntax ERROR in "+filename+lua_tostring(L,-1);
        ret=1;
    }
	else m_error.clear();
    fclose(file);
    return ret;
}

Var VMCallable::eval(const string & s) {
	int duplicated=0;
	if(!s.size()) {
		lua_pushvalue(L, -1); // duplicate script on stack for reuse
		duplicated = 1;
	}
	else if (luaL_loadbuffer(L, s.c_str(), s.size(), 0)) {
        m_error="Syntax ERROR in ";
		m_error+=lua_tostring(L,-1);
		lua_settop(L,-2);
		return Var::null;
	}
	if(lua_pcall(L,0,0,0)!=0) {
        m_error="Runtime ERROR in ";
		m_error+=lua_tostring(L,-1);
		lua_settop(L,-2);
		return Var::null;
	}
	m_error.clear();
	int argc = lua_gettop(L);
	if(argc<=duplicated) return Var::null;
	else if(argc==duplicated+1) {
		Var ret = lua2var(L, -1);
		lua_pop(L,1);
		return ret;
	}
	Var ret;
	for(int i=duplicated+1; i<=argc; ++i)
		ret.append(lua2var(L, i));
	lua_settop(L, duplicated);
	return ret;
}

Var VMCallable::call(const std::string & cmd, const Var & arg) {
	int counter = push(L, cmd.c_str());
	if(!counter) return Var::null;
	int argc;
	if(arg.type()==Var::ARRAY) {
		argc = arg.size();
		for(int i=0; i<argc; ++i)
			if(!arg[i].type()) lua_pushnil(L);
			else var2lua(L, arg[i]);
	}
	else argc = var2lua(L, arg);

	if(lua_pcall(L, argc, 1, 0)!=0) {
        m_error = "Runtime ERROR in Lua function "+cmd+": "+lua_tostring(L,-1);
		lua_settop(L,-2);
		if(counter>1) lua_pop(L,counter-1);
		return Var::null;
	}
	m_error.clear();
	Var ret = lua2var(L, -1);
	lua_pop(L,counter);
	return ret;
}

void VMCallable::set(const string & name, const Var & v) {
	if(var2lua(L, v)) lua_setglobal(L, name.c_str());
}


Var VMCallable::get(const string & name) {
	int counter = push(L, name.c_str());
	if(!counter) return Var::null;
	Var v = lua2var(L, -1);
	lua_pop(L,counter);
	return v;
}

int VMCallable::Callable_call(lua_State *L) {
	// check type:
	const char* basetype = 0;
	if(lua_isuserdata(L,1)&&lua_getmetatable(L,1)) {
		lua_pushstring(L, "basetype");
		lua_gettable(L, -2);
		basetype = lua_tostring(L, -1);
	}
	if(!basetype||(strcmp(basetype, "Callable")!=0)) {
		VMCallable* pInstance = instance(L);
		if(pInstance) pInstance->m_error="Callable userdata expected as argument 1";
		lua_pop(L,2); // metatable and basetype
		return 0;
	}
	lua_pop(L,2); // metatable and basetype
	
	Callable* pCallable = Callable::instance((char*)lua_touserdata(L,1));
	if(!pCallable) return 0;
	const char * cmd = lua_tostring(L, lua_upvalueindex(1));
	Var arg;
	for(int i=2; i<=lua_gettop(L); ++i)
		arg.append(lua2var(L,i));
	return var2lua(L, pCallable->call(cmd, arg));
}

int VMCallable::Callable_info(lua_State *L) {
	// check type:
	const char* basetype = 0;
	if(lua_isuserdata(L,1)&&lua_getmetatable(L,1)) {
		lua_pushstring(L, "basetype");
		lua_gettable(L, -2);
		basetype = lua_tostring(L, -1);
	}
	if(!basetype||(strcmp(basetype, "Callable")!=0)) {
		VMCallable* pInstance = instance(L);
		if(pInstance) pInstance->m_error="Callable userdata expected as argument 1";
		lua_pop(L,2); // metatable and basetype
		return 0;
	}
	lua_pop(L,2); // metatable and basetype
	
	Callable* pCallable = Callable::instance((char*)lua_touserdata(L,1));
	if(!pCallable) return 0;
	Var ret(pCallable->info());
	ret.append("info");
	return var2lua(L, ret);
}

void VMCallable::bind(const string & name, Callable & callable) {
	callable.name(name);
	strcpy((char*)lua_newuserdata(L, name.size()+1), name.c_str());
	if(luaL_newmetatable(L, ("Callable."+name).c_str() )) { // define metamethods
		lua_pushstring(L, "basetype"); // common marker for identifying Callables
		lua_pushstring(L, "Callable"); 
		lua_settable(L, -3); 

		lua_pushstring(L, "__index");
		lua_pushvalue(L, -2); 
		lua_settable(L, -3); 
		
		// register methods:
		lua_pushstring(L, "info");
		lua_pushcfunction(L, Callable_info);
		lua_settable(L, -3); 	
		
		Var methods = callable.info();
		for(unsigned int i=0; i<methods.size(); ++i) {
			lua_pushstring(L, methods[i].string().c_str());
			lua_pushstring(L, methods[i].string().c_str());
			lua_pushcclosure(L, Callable_call, 1);
			lua_settable(L, -3); 	
		}
	}
	lua_setmetatable(L,-2);
	lua_setglobal(L,name.c_str());
}

bool VMCallable::unbind(Callable & callable) {
	lua_getglobal(L,callable.name().c_str());
	// check type:
	if(!lua_isuserdata(L,-1)||!lua_getmetatable(L,-1)) {
		lua_pop(L,1);
		return false;
	}
	lua_pushstring(L, "basetype");
	lua_gettable(L, -2);
	const char* basetype = lua_tostring(L, -1);
	if(!basetype||(strcmp(basetype, "Callable")!=0)) {
		lua_pop(L,2); // metatable and basetype
		return false;
	}
	lua_pop(L,2); // metatable and basetype
	// unbind:
	lua_pushnil(L);
	lua_setglobal(L,callable.name().c_str());
	return true;
}

VMCallable* VMCallable::instance(lua_State *L) {
	map<lua_State*,VMCallable*>::iterator it = sm_vm.find(L);
	return it==sm_vm.end() ? 0 : it->second;
}

