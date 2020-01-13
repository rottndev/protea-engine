#ifndef _PRO_MATERIAL_H
#define _PRO_MATERIAL_H

/** @file proMaterial.h
 \brief contains classes holding and managing material data

 \author  Gerald Franz, www.viremo.de
 
 $Revision: 1.3 $
 current version 2008-05-29

 License notice (zlib license):

 (c) 2006-2008 by Gerald Franz, www.viremo.de

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

#include <string>
#include <vector>

#include "proXml.h"
#include "proMath.h"

//--- class proMatData ---------------------------------------------

/// internal class storing information about a conventional OpenGL material
class proMatData {
public:
    /// proMaterial class is allowed to access all data members
    friend class proMaterial;
protected:
    /// default constructor
    proMatData(const std::string & name="");
    /// returns xml statement containing all data
    Xml xml() const;
    /// returns material as VRML Appearance node
    std::string vrml(unsigned int nTabs=0) const;
    /// comparison operator equality, name is disregarded
    bool operator==(const proMatData &mat) const;

    /// stores name
    std::string m_name;
    /// stores color information
    vec4f m_color;
    /// stores ambient color
    vec4f m_ambient;
    /// stores specular color
    vec3f m_specular;
    /// stores shininess
    float m_shininess;
    /// stores emissive color
    vec3f m_emissive;

    /// stores texture filename
    std::string m_url;
    /// stores texture id
    unsigned int m_texId;
    /// stores texture scale
    vec2f m_texScale;
    /// stores texture repetiveness in two directions
    bool m_texRepeat[2];
    /// stores texture transparency
    bool m_texTransparent;

    /// stores reference counter
    unsigned int m_refCount;

    /// global default material in case no other is set
    static proMatData s_default;
};


//--- class proMaterial ---------------------------------------------
/// a class managing and offering public access to shared OpenGL material data
/** In order to make the application of this class most convenient, it acts as a smart pointer
to a proMatData object. */
class proMaterial {
public:
    /// default constructor
    proMaterial() {
        m_data=&proMatData::s_default; }
    /// constructor providing a name
    proMaterial(const std::string & name) {
        m_data=new proMatData(name); }
    /// copy constructor incrementing reference count
    proMaterial(const proMaterial & source);
    /// constructor from an xml statement
    proMaterial(const Xml & xs);
    /// destructor
    ~proMaterial();
    /// copy operator incrementing reference count
    const proMaterial & operator=(const proMaterial & source);
        
    /// duplicates a given material, produces a physical copy
    void set(const proMaterial & source);
    /// sets data by an xml statement
    void set(const Xml & xs);
        
    /// returns xml statement containing all data
    Xml xml() const { return m_data->xml(); }
    /// returns material as VRML Appearance node
    std::string vrml(unsigned int nTabs=0) const { return m_data->vrml(); }
    /// returns name
    const std::string & name() const { return m_data->m_name; }
    /// sets name
    void name(const std::string & s) { m_data->m_name=s; }
    /// comparison operator equality, name is disregarded
    bool operator==(const proMaterial &mat) const { return m_data->operator==(*mat.m_data); }
    /// comparison operator inequality, name is disregarded
    bool operator!=(const proMaterial &mat) const { return operator==(mat)? false : true; }

    /// sets (diffuse) RGBA color
    /** The fourth argument determines the material's alpha (i.e. 1-transparency) */
    void color ( float r, float g, float b, float a=1.0f ) {
		m_data->m_color.set(r,g,b,a); }
    /// sets (diffuse) RGBA color from a vec4f
    void color (const vec4f & col) { m_data->m_color=col; }
    /// returns (diffuse) RGBA color
    /** The fourth argument determines the material's alpha (i.e. 1-transparency) */
    const vec4f & color () const { return m_data->m_color; }
    /// returns ambient color
    /** Since this factor is without any direct physical correspondence, it should be  rather left untouched. 
	 If no ambient color has been explicitly set, the diffuse color is returned. */
	const vec4f & ambientColor () const { return m_data->m_ambient[0]<0.0f ? m_data->m_color : m_data->m_ambient; }
    /// sets ambient color
    /** Since this factor is without any direct physical correspondence,  it should be  rather left untouched. */
	void ambientColor (const vec3f & c) { m_data->m_ambient[0]=c[0]; m_data->m_ambient[1]=c[1]; m_data->m_ambient[2]=c[2]; }
    /// returns specular color
    /** Since this factor is without any direct physical correspondence,  it might be best 
    either set to 1.0 1.0 1.0 (corresponding to a plastic-like material) or to the mean of light 
    color and normal diffuse color (corresponding to a metal-like material). */
    const vec3f & specularColor () const { return m_data->m_specular; }
    /// sets specular color
    /** Since this factor is without any direct physical correspondence,  it might be best 
    either set to 1.0 1.0 1.0 (corresponding to a plastic-like material) or to the mean of light 
    color and normal diffuse color (corresponding to a metal-like material). */
    void specularColor (const vec3f & c) { m_data->m_specular=c; }
    /// returns shininess
    /** Since this factor is without any direct physical correspondence,  it might be best 
    interpreted as specularity factor, which determines the fraction of radiance reflected direction-dependently. */
    float shininess() const { return m_data->m_shininess; }
    /// sets shininess
    /** Since this factor is without any direct physical correspondence,  it might be best 
    interpreted as specularity factor, which determines the fraction of radiance reflected direction-dependently. */
    void shininess(float f) { m_data->m_shininess=f; }
    /// returns emissive color
    /** Since this factor is without any direct physical correspondence,  it should be
     rather replaced by a uniform emissiveness factor. */
    const vec3f & emissiveColor () const { return m_data->m_emissive; }
    /// sets emissive color
    /** Since this factor is without any direct physical correspondence,  it should be rather replaced by a uniform emissiveness factor. */
    void emissiveColor (const vec3f & c) { m_data->m_emissive=c; }
    /// returns true if material is (at least a bit) transparent.
    bool transparent() const { return (m_data->m_color[3]<1.0f) || m_data->m_texTransparent; }

    /// returns texture url
    const std::string & texName() const { return m_data->m_url; }
    /// sets name
    void texName(const std::string & s) { m_data->m_url=s; }
    /// returns texture id
    unsigned int texId() const { return m_data->m_texId; }
    /// initiates a load if necessary and sets texture id and texture transparency. Returns texture ID
    unsigned int loadTexture(bool reload=false);
    /// returns texture scale
    const vec2f & texScale() const { return m_data->m_texScale; }
    /// sets texture scale
    void texScale(const vec2f & sc) { m_data->m_texScale=sc; }
    /// returns texture repetiveness
    bool texRepeat(unsigned int n=0) const { return m_data->m_texRepeat[n%2]; }
 
protected:
	/// pointer to proMatData
	proMatData * m_data;
};


//--- class MaterialMgr --------------------------------------------
/// singleton class managing material definitions
/** Materials currently cannot be erased individually, because this may screw up the current access system based on direct array indices. */
class MaterialMgr {
public:
    /// returns singleton instance
    static MaterialMgr & singleton() {
        if(!sp_instance) sp_instance=new MaterialMgr; return *sp_instance; }
    /// returns pointer to singleton instance
    static MaterialMgr * singletonPtr() {
        if(!sp_instance) sp_instance=new MaterialMgr; return sp_instance; }
        
    /// clears material table
    void clear() { m_vMat.clear(); m_vMat.push_back(proMaterial()); }
    /// returns number of material entries
    size_t size() const { return m_vMat.size(); }

    /// reads multiple material definitions from a <Materials/> xml table
    void interpret(const Xml & xs);
    /// returns table as xml statement
    Xml xml() const;

    /// sets a material in the table
    /** In case that no material having the same name exists, a new material is added.
     \param mat the material to be set
     \return id of the added material. */
    size_t set(const proMaterial & mat);
    /// adds a material to the table
    /** in case that a material having the same name already exists, this method does nothing. 
     \param mat the material to be added
     \return id of the added material. */
    size_t add(const proMaterial & mat);
    /// adds an anonymous material to the table and generates a name
    /** in case that a material having the same properties exists, this material is reused. 
     \param mat the material to be added
     \return id of the added material. */
    size_t addAnonymous(const proMaterial & mat);
    /// returns material by its name or the default material if name is not found
    proMaterial & operator[](const std::string & s);
    /// returns material by its name or the default material if name is not found, const
    const proMaterial & operator[](const std::string & s) const;
    /// returns material by its numerical id
    proMaterial & operator[](size_t n) { return m_vMat[(n<m_vMat.size()) ? n : 0]; }
    /// returns material by its numerical id, const
    const proMaterial & operator[](size_t n) const { return m_vMat[(n<m_vMat.size()) ? n : 0]; }
	/// returns default material
    const proMaterial & defaultMat() const  { return m_vMat[0]; }
    /// returns material id by its name or the default material's id (0) if name is not found
    unsigned int getId(const std::string & s) const;
	/// reloads all textures and further resources
	void reload() {
		for(unsigned int i=0; i<m_vMat.size(); ++i) m_vMat[i].loadTexture(true); }
protected:    
    /// constructor defining a default material
    MaterialMgr() : m_counter(0) { m_vMat.push_back(proMaterial()); }
    /// pointer to singleton instance
    static MaterialMgr* sp_instance;
    /// table for storing material data
    std::vector<proMaterial> m_vMat;
    /// counter for generating material names
    unsigned int m_counter;
};

#endif // _PRO_MATERIAL_H
