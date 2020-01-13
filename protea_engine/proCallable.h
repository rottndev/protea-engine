/** @file proCallable.h
 \brief A framework for the flexible integration of C++ classes with Lua at runtime

 \author Gerald Franz, www.viremo.de
current version 2009-07-29

  License notice (zlib license):

 (c) 2007-2009 by Gerald Franz, www.viremo.de

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _PRO_CALLABLE_H
#define _PRO_CALLABLE_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

class Var;    

//--- class Callable -----------------------------------------------
/// abstract base class allowing objects to register themselves as target for interpreted commands
class Callable {
public:
	/// default constructor
	Callable() { }
	/// copy constructor
	Callable(const Callable & source) : m_name(source.m_name) { 
		if(m_name.size()) sm_instance[m_name]=this; }
	/// destructor unregistering object instance
	virtual ~Callable();
	/// copy operator
	const Callable & operator=(const Callable & source);    
	/// generic method calling the object instance to evalute the provided command, interface definition
	virtual Var call(const std::string & cmd, const Var & arg)=0;
	/// returns all keys/command names provided by this Callable as Var::ARRAY
	virtual Var info() const;
	/// sets name, normally implicitly done by binding to a virtual machine
	void name(const std::string & s);
	/// returns name
	const std::string & name() const { return m_name; }
	/// returns pointer to instance by name
	static Callable* instance(const std::string & name);
protected:
	/// stores name given by CallVM::reg() method
	std::string m_name;
	/// map associating names with object addresses
	static std::map<std::string,Callable*> sm_instance;
};

//--- class Var ----------------------------------------------------
/// a generic and flexible variable class
class Var {
public:
	/// default constructor
	Var() : m_type(NONE) { u.i=0; }
	/// constructor from an integer
	Var(int i) : m_type(INT) { u.i=i; }
	/// constructor from an unsigned integer
	Var(unsigned int ui) : m_type(INT) { u.i=static_cast<int>(ui); }
	/// constructor from a float
	Var(float f) : m_type(FLOAT) { u.f=f; }
	/// constructor from a double
	Var(double d) : m_type(FLOAT) { u.f=static_cast<float>(d); }
	/// constructor from a string
	Var(const std::string & s);
	/// constructor from a const char*
	Var(const char * s);
	/// copy constructor
	Var(const Var & source);
	/// destructor
	virtual ~Var();

	/// copy operator
	const Var & operator=(const Var & source);
	/// equality test operator
	bool operator==(const Var & v) const;
	/// inequality test operator
	bool operator!=(const Var & v) const {
        return !operator==(v); }
	/// array element access operator
	/** \warning returns Var::null in case of an illegal array index
	  \warning index access to Var::MAP objects is not yet supported */
	Var & operator[](int n);
	/// array element access operator, const
	/** \warning returns Var::null in case of an illegal array index
	  \warning index access to Var::MAP objects is not yet supported */
	const Var & operator[](int n) const;
	/// hash element access operator
	/** \warning returns Var::null in case of an illegal hash key */
	Var & operator[](std::string key);
	/// hash element access operator, const
	/** \warning returns Var::null in case of an illegal hash key */
	const Var & operator[](std::string key) const;

	/// returns type id
	unsigned int type() const { return m_type; }
	/// returns size of Variable, result depends on type
    /** If the object of type string, the length of the string is returned. 
         If the object is of type ARRAY, the number of elements is returned.
         In all other cases 0 is returned.
	  \warning index access to Var::MAP objects is not yet supported */
	unsigned int size() const;
	/// returns all key names provided as Var::ARRAY if this Var is of type MAP
	Var info() const;

	/// appends a subvar, only allowed for void or ARRAY type
	Var & append(const Var & v);
	/// sets a key-value pair, only allowed for void or MAP type
	Var & set(const std::string & key, const Var & v);
	/// erases a subvar identified by its index
	/** \return 0 in case of success */
	int erase(int n);
	/// erases a subvar identified by its key
	/** \return 0 in case of success */
	int erase(const std::string & key);
    /// resets variable to an empty void state
    void clear();

	/// returns content as int if possible
	int integer() const;
	/// returns content as float number if possible
	float number() const;
	/// returns content as string if possible
	std::string string() const;
	/// evaluates content as bool
	/** Vars of type none, array, map, and float and int values of 0.0, and strings different from "true" are evaluated as false, the rest is true */
	bool boolean() const;
	/// returns a string representation of the content
	std::string repr() const;
	/// returns content as numeric array if possible
	size_t numArray(float* arr, size_t size) const;
    /// interprets a string representation of a Var
    /** \warning This method is neither complete nor robust. */
    static Var interpret(std::string s);
    
	/// defines symbolic names for possible variable types
	enum {
		NONE = 0,
		INT = 1,
		FLOAT = 2,
		NUM = INT|FLOAT,
		STRING = 4,
		ARRAY = 8,
		MAP = 16,
	};

	/// static Var object indicating an illegal return value
	static Var null;
protected:
	/// stores type
	unsigned short m_type;
	/// stores content in a union
    union {
        int i;
        float f;
        std::string *s;
        std::vector<Var> *v;
        std::map<std::string, Var> *m;
    } u;
};

/// operator for output of Var objects to streams
std::ostream & operator<<(std::ostream & os, const Var & v);

//--- class VMCallable ---------------------------------------------
struct lua_State;
/// Lua based Callable virtual machine and adapter class
class VMCallable {
public:
	/// default constructor creating a Lua state and optionally opening default libraries
	VMCallable(bool openDefaultLibs=true);
	/// destructor closing Lua state
	~VMCallable();

	/// sets status
	void status(int i) { m_status=i; }
	/// returns status
	int status() const { return m_status; }
	/// loads a lua script from a file
	int load(const std::string & filename);
	/// evaluates a Lua string or the previously defined/loaded script
	Var eval(const std::string & s=std::string());
	/// calls a global or nested function defined in Lua
	Var call(const std::string & cmd, const Var & arg=Var::null);
	/// returns error message generated by last load()/eval() call
	/** \return error string. An empty string indicates that no error has occurred */
	const std::string & error() const { return m_error; }
	/// returns internal Lua state
	lua_State* state() { return L; }
	/// returns a global or nested lua variable as Var object
	Var get(const std::string & name);
	/// defines a named global Lua variable from a Var object
	void set(const std::string & name, const Var & v);
	/// binds a Callable object to the lua VM and defines a global name
	void bind(const std::string & name, Callable & callable);
	/// releases the bindings of a Callable object to the lua VM
	bool unbind(Callable & callable);
	/// allows retrieving a virtual machine by its Lua state 
	static VMCallable* instance(lua_State* L);
protected:
	/// pointer to Lua state
	lua_State* L;
	/// stores status
	/** currently a status of -1 indicates a controlled shutdown request*/
	int m_status;
	/// error string
	std::string m_error;
	/// map associating Lua states with virtual machines
	static std::map<lua_State*,VMCallable*> sm_vm;
	/// Callable call adapter
	static int Callable_call(lua_State *L);
	/// Callable info adapter
	static int Callable_info(lua_State *L);
};

#endif //_PRO_CALLABLE_H
