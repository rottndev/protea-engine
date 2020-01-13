#ifndef _PRO_XML_H
#define _PRO_XML_H

/** @file proXml.h

 \brief contains the class Xml
 \author  Gerald Franz, www.viremo.de
 \version 2.1
 
  License notice (zlib style):

 (c) 2005-2009 by Gerald Franz, www.viremo.de

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

#include <vector>
#include <string>

/// a class implementing a document object model (DOM) of an xml file
class Xml {
public:
    /// default constructor
    Xml(const std::string & tag="tag") : m_tag(tag), mp_parent(0) { 
        if(!sv_code.size()) registerDefaultCodes(); }
    /// copy constructor
    Xml(const Xml &source);
    /// destructor
    ~Xml();
    /// copy operator=
    Xml &operator=(const Xml & source);
		
	/// comparison operator equality
	bool operator==(const Xml & xs) const;
	/// comparison operator inequality
	bool operator!=(const Xml & xs) const { return !operator==(xs); }

    /// sets tag name
    void tag(const std::string & name) { m_tag=name; }
    /// returns tag name
    const std::string & tag() const { return m_tag; }
    /// returns address of parent statement or NULL if toplevel
    Xml * parent() const { return mp_parent; }

    /// returns attribute value as string. If it does not exist, an empty string is returned.
    std::string attr(const std::string & name) const;
    /// returns a pair consisting of key and value of attribute n as string. If it does not exist, an empty string is returned.
    std::pair<std::string,std::string> attr(size_t n) const {
        return (n*2+1>mv_attr.size()) ? std::make_pair(std::string(),std::string()) : std::make_pair(mv_attr[n*2],mv_attr[n*2+1]); }
    /// sets an attribute value. If it does not exist, a new attribute is added. If an empty value is passed, the attribute is removed.
    void attr(const std::string & name, const std::string & value);
    /// returns number of attributes
    size_t nAttr() const { return mv_attr.size()/2; }

    /// appends a copy of xmlSt as new child statement
	/** \param xmlSt the Xml statement to be copied and appended
	\return a reference to the copied statement */
    Xml & append(const Xml & xmlSt);
    /// appends a string to the content
    void append(const std::string & s);
    /// appends a copy of xmlSt as new child statement
	/** syntactic sugar to allow nicely chained appends,
	\param xmlSt the Xml statement to be copied and appended
	\return a reference to this statement */
    Xml & operator<<(const Xml & xmlSt) { append(xmlSt); return *this; }
    /// appends a string to the content
	/** syntactic sugar to allow nicely chained appends,
	\param s the string to be copied and appended
	\return a reference to this statement */
    Xml & operator<<(const std::string & s) { append(s);  return *this; }
	
    /// returns number of content elements
    size_t nChildren() const { return mv_elem.size(); }
    /// returns the nth child element
    std::pair<Xml*,std::string> child(size_t number) { return mv_elem[number]; }
    /// returns the nth child element
    const std::pair<Xml*,std::string> child(size_t number) const  {
        return mv_elem[number]; }
    /// returns a specified direct child statement or NULL
    /** \param  tagName the tag of the searched statement. Pass an empty string "" to select only by attribute 
        \param attrKey (optional) the key of a specified attribute
        \param attrValue (optional) the value of a specified attribute */
    Xml * child(const std::string & tagName, const std::string & attrKey="", const std::string & attrValue="");
    /// returns a specified direct child statement or NULL, const
    /** \param  tagName the tag of the searched statement. Pass an empty string "" to select only by attribute 
        \param attrKey (optional) the key of a specified attribute
        \param attrValue (optional) the value of a specified attribute */
    const Xml * child(const std::string & tagName, const std::string & attrKey="", const std::string & attrValue="") const;
    /// returns a pointer to the first (sub)statement with suitable tag and optionally attribute, or NULL if none is found
    /** \param  tagName the tag of the searched statement. Pass an empty string "" to select only by attribute 
        \param attrKey (optional) the key of a specified attribute
        \param attrValue (optional) the value of a specified attribute */
    Xml * find(const std::string & tagName, const std::string & attrKey="", const std::string & attrValue="");
    /// returns a pointer to the first (sub)statement with suitable tag and optionally attribute, or NULL if none is found, const
    /** \param  tagName the tag of the searched statement. Pass an empty string "" to select only by attribute 
        \param attrKey (optional) the key of a specified attribute
        \param attrValue (optional) the value of a specified attribute */
    const Xml * find(const std::string & tagName, const std::string & attrKey="", const std::string & attrValue="") const;
    /// erases child statement
    int erase(Xml * xmlSt);

    /// clears all existing information.
    void clear();
    /// loads xml from disk
    /** \param filename url of the file to be loaded
     \return a new Xml object containing the loaded data. */
    static Xml load(const std::string & filename);
    /// writes xml to disk.
    int save(const std::string & filename) const;
    /// evaluates a string as xml, previous information is cleared.
    void eval(const std::string & s);
    /// returns a preformatted xml string
    std::string str(unsigned int nTabs=0) const;
    
    /// registers a custom code for special character conversion
    /** call this method before any Xml constructor in case the default table shall be overwritten, call it afterwards to keep it */
    static void registerCode(const std::string & decoded, const std::string & encoded);
protected:
    /// stores xml tag
    std::string m_tag;
    /// stores xml attributes, keys have even indizes, values odd indizes.
	std::vector<std::string> mv_attr;
    /// stores pointers to subordinate statements or strings
	std::vector<std::pair<Xml*,std::string> > mv_elem;
    /// stores pointer to superordinate statement
    Xml * mp_parent;

    /// encodes special characters in xml
    static std::string encode(std::string s);
    /// decodes xml encoded special characters to ascii
    static std::string decode(std::string s);
    /// registers standard entries in code table for special characters
    /** automatically called by first constructor in case the code table is still empty */
    static void registerDefaultCodes();
    /// stores code table for special characters
    static std::vector<std::pair<std::string,std::string> > sv_code;
};

/// operator for output of Xml objects to streams
inline std::ostream & operator<<(std::ostream & os, const Xml & xml) { return (os << xml.str()); }

#endif
