#ifndef _PRO_SCENE_H
#define _PRO_SCENE_H

/** @file proScene.h
 \brief  a minimal scene graph
 \version 2009-07-11

 License notice (zlib license):

 (c) 2006-2009 by Gerald Franz, www.viremo.de

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

#include "proXml.h"
#include "proMath.h"
#include "proMaterial.h"
#include <vector>

class Renderer;
class Renderable;

/// symbolic names for proNode flags
enum flag_t {
    /// stores whether object is active
    FLAG_ACTIVE=1<<0,
    /// stores whether object needs an update
    FLAG_UPDATE=1<<1,
    /// stores whether object is transparent  / whether traversal  addresses transparent objects
    FLAG_TRANSPARENT=1<<2,
    /// stores whether object casts shadow  / whether traversal addresses shadows
    FLAG_SHADOW=1<<3,
    /// stores whether object shadow uses the zfail method
    FLAG_ZFAIL=1<<4,
    /// stores whether object  is a light / whether traversal addresses lights
    FLAG_LIGHT=1<<5,
    /// stores whether object contains or traversal addresses renderable geometry
    FLAG_RENDER=1<<6,
    /// stores whether object contains or traversal addresses collision geometry
    FLAG_COLLISION=1<<7,
	/// flag indicating current traversal / object visualization shall be done as wireframe
	FLAG_WIREFRAME = 1<<8,
	/// flag indicating current traversal / object visualization shall render front and backfaces
	FLAG_FRONT_AND_BACK = 1<<9,
};

class proLight;
class proCamera;

//--- class proNode -------------------------------------------------

/// base class for all derived geometry and scene graph nodes.
class proNode {
public:
	/// an iterator class for proNode based scene graphs
	/** Example :\n
	  \code
		for(proNode::iterator iter=&scene; iter!=0; ++iter)
			cout << iter->xml().tag() << " " << iter->name() << endl;
	  \endcode
	*/
	class iterator {
	public:
		/// constructor from a proNode pointer
		iterator(proNode * pObject) : mp_obj(pObject) { }
		/// assignment operator
		void operator=(proNode * pObject) { mp_obj=pObject; mv_stack.clear(); }
		/// comparison operator inequality
		bool operator!=(proNode * pObject) { return mp_obj!=pObject; }
		/// element operator, returns pointer to current object
		proNode * operator->() const { return mp_obj; }
		/// element operator, allows accessing pointer to current object
		proNode * & operator->() { return mp_obj; }
		/// indirection operator, returns pointer to current object
		proNode * operator*() const { return mp_obj; }
		/// increment operator, moves iterator to next object address
		void operator++() { 
			if(mp_obj) mp_obj=mp_obj->next(*this);
			while(!mp_obj&&mv_stack.size()) mp_obj=mv_stack.back().first->next(*this); }

		/// pushes stack
		void push(proNode * pObj) { 
			if(mv_stack.size()&&(mv_stack.back().first==pObj)) ++mv_stack[mv_stack.size()-1].second;
			else mv_stack.push_back(std::make_pair(pObj,0)); }
		/// pops stack
		void pop() { mv_stack.pop_back(); }
		/// returns stack top object pointer
		proNode * topObj() { return mv_stack.size() ? mv_stack.back().first : 0; }
		/// returns stack top counter
		size_t topCounter() { return mv_stack.size() ? mv_stack.back().second : 0; }
		/// returns stack size
		size_t stackSize() const { return mv_stack.size(); }
	protected:
		/// pointer to current object
		proNode * mp_obj;
		/// stack of indices for recursively iterating through scene graphs
		std::vector< std::pair<proNode*, size_t> > mv_stack;
	};

    /// default constructor, optional argument is user definable name.
    proNode(const std::string & name="") : m_bndSphere(0.0f,0.0f,0.0f,-1.0f), m_name(name), m_flags(FLAG_ACTIVE|FLAG_UPDATE), m_queryFlags(0), mp_renderable(0) { }
    /// copy constructor
    proNode(const proNode & source) : m_bndSphere(source.m_bndSphere), m_name(source.m_name), m_flags(source.m_flags), m_queryFlags(source.m_queryFlags), mp_renderable(0) { }
    /// destructor
    virtual ~proNode() { }
    /// returns a pointer to a copy of the object
    /**\return a pointer to a copy.
	This method has to be redefined by all descendants in order to allow parents to copy scenegraph branches recursively. */
    virtual proNode * copy() const =0;

    /// returns individual name
    const std::string & name() const { return m_name; }
    /// sets individual name
    void name( const std::string & s ) { m_name=s; }
    /// returns the node type
    virtual std::string type() const { return TYPE; }
	/// type name
	static const char* const TYPE;
	
    /// returns flags
    unsigned int flags() const { return m_flags; }
    /// allows accessing flags
    /** avoid this function whenever possible, since it does not propagate to children */
    unsigned int & flags() { return m_flags; }
    /// sets one or more flags
    virtual void enable(unsigned int flag) { m_flags|=flag; }
    /// unsets one or more flags
    virtual void disable(unsigned int flag) { m_flags&=~flag; }
    /// returns query flags
    unsigned int queryFlags() const { return m_queryFlags; }
    /// sets query flags
	/** When performing a scene query, an object is included or excluded depending on bitwise matches between its query flags and the query's query flags.  */
    void queryFlags(unsigned int qf) { m_queryFlags=qf; }

    /// updates object
    virtual void update();
    /// performs a single render pass according to the provided camera and context by calling the associated Renderable object
    /** \param camera current camera settings*/
    virtual void draw(proCamera & camera);
    /// performs graphics initializations, interface definition.
    /** OpenGL initializations can only be done as soon as an OpenGL window is opened.
     Since proNodes might be useful without window (e.g., collision), all OpenGL operations are restricted to the draw(),
     initGraphics(), and closeGraphics() methods. */
    virtual void initGraphics();
    /// cleans up graphics initializations, interface definition.
    virtual void closeGraphics();
    /// allows iteration of proNode graphs
    virtual proNode * next(proNode::iterator & iter) { return 0; }

    /// transforms this object by multiplying it with matrix m, interface definition.
    virtual void transform(const mat4f &) { }

    /// computes the bounding geometry.
    /** interface definition, not for realtime! */
    virtual void calcBounding(bool recursive=true) { }
    /// tests for intersection of the bounding geometry with a passed frustum, automatically called by draw()
    /** \param frust the transformed camera frustum
     \param matrix current transformation matrix
     \return true if the bounding geometry is intersected, otherwise false. */
    virtual bool testBounding(const frustum & frust, const mat4f & matrix) const;
    /// returns the bounding sphere
    /**  a radius<0.0f is interpreted as always draw */
    virtual sphere boundingSphere() const { return m_bndSphere; }
    /// returns the axis-aligned bounding box
    virtual const std::pair<vec3f,vec3f> & boundingBox() const { return m_bbox; }
    /// tests for intersection with ray
    virtual bool intersects(const line & ray) const { return false; }
    /// returns intersection point with ray or null in case of none
    virtual vec3f* intersection(const line & ray) const { return 0; }
    /// returns pointer to nearest (sub-)node which is intersected by the passed ray
	/** \param ray infinite ray
	  \param queryFlags (optional, default all) When performing a scene query, an object is included or excluded depending on bitwise matches between its query flags and the query's query flags.  */
    virtual const proNode * query(const line & ray, unsigned int queryFlags=0xFFFFFFFF) const { return 0; }

    /// returns object as xml statement
    virtual Xml xml() const;   
    /// interprets an X3D xml statement as proNodes
    static proNode * interpret(const Xml & xs);
	/// sets global renderer
	/** should be done before initializing proNode children instances */
	static void renderer(Renderer * pRenderer) { sp_renderer = pRenderer; }
	/// returns global renderer
	static Renderer *  renderer() { return sp_renderer; }
protected:
    /// stores bounding sphere
    sphere m_bndSphere;
    /// stores axis-aligned bounding box
    std::pair<vec3f,vec3f> m_bbox;
    /// stores name
    std::string m_name;
    /// stores flags
    unsigned int m_flags;
    /// stores queryflags
    unsigned int m_queryFlags;

	/// pointer to corresponding Renderable object
	Renderable * mp_renderable;
	/// stores pointer to global renderer object
	static Renderer* sp_renderer;
};

//--- class proCamera -----------------------------------------------
/// a class encapsulating observer specific rendering information and OpenGL commands
class proCamera : public proNode {
public:
    /// default constructor
    proCamera();
    /// returns a pointer to a physical copy of the object
    virtual proNode * copy() const { return new proCamera(*this); }

    /// sets the current time
    void time(double tNow) { m_tNow=tNow; }
	/// returns the current time 
	double time() const { return m_tNow; }
    /// returns the current camera position
	const vec6f & pos() const { return m_pos; }
    /// allows access to the current camera position
	vec6f & pos() { return m_pos; }
    /// returns the current frustum dimensions
	const vec6f & dim() const { return m_dim; }
    /// allows accessing the current frustum dimensions
	/** note that, unlike OpenGL, in protea the near clipping distance does not automatically affects the position of the left/right/upper/lower clipping planes */
	vec6f & dim() { return m_dim; }
	/// returns the current frustum
	const frustum & frs() const { return m_frustum; }
    /// returns flags
    unsigned int flags() const { return m_flags; }
    /// allows accessing flags
    unsigned int & flags() { return m_flags; }
    /// returns pointer to currently active light
    proLight * light() const { return m_pLight; }
    /// allows changing currently active light
    void light(proLight * pLight) { m_pLight=pLight; }
    /// converts 2D camera coordinates (normalized to -1|1) to 3D ray in world coordinates depending on current camera position and frustum
    line ray(float relX=0.0f, float relY=0.0f) const;
    
	/// returns current transformation matrix
	mat4f & matrix() { return m_mat[m_currMat]; }
    /// pushs current matrix and multiplies it with provided matrix
    void push(const mat4f & matrix);
    /// pops current matrix and model view matrix
    void pop();
    /// initializes graphics
    virtual void initGraphics() { };
    /// applies current camera settings to rendering context
    virtual void update();
	
	/// calculates and returns the camera's direction vector in world coordinates
	vec3f direction() const;
	/// calculates and returns the camera's right vector in world coordinates
	vec3f right() const;
	/// calculates and returns the camera's up vector in world coordinates
	vec3f up() const;

    /// returns the node type
    std::string type() const { return TYPE; }
	/// type name
	static const char* const TYPE;
protected:
    /// stores current time
	double m_tNow;
	/// stores camera position and orientation
	vec6f m_pos;
	/// stores  the current frustum dimensions
	vec6f m_dim;
	/// stores the current frustum planes
	frustum m_frustum;
    /// stores number of provided matrices
    static const unsigned int s_nMat=32;
	/// stores transformations
	mat4f m_mat[s_nMat];
    /// stores id of current matrix
    unsigned int m_currMat;
    /// stores flags
    unsigned int m_flags;
    /// stores currently dominant light (e.g., the one that casts shadows)
    proLight * m_pLight;
	/// pointer to corresponding Renderable object
	Renderable * mp_renderable;
};

//--- class proLight ------------------------------------------------
/// a class representing a light source (point light/distant light)
class proLight : public proNode {
public:
    /// constructor
    proLight(const vec4f & position);
    /// constructor interpreting an X3D defined light node.
    proLight(const Xml & xs);
    /// returns a pointer to a physical copy of the object
    virtual proNode * copy() const { return new proLight(*this); }

    /// updates object
    virtual void update();
    /// returns the node type
    virtual std::string type() const { return TYPE; }
	/// type name
	static const char* const TYPE;
    
    /// returns light position
	const vec4f & pos() const { return m_pos; }
    /// sets light position
	void pos(const vec4f & position) { m_pos=position; m_flags|=FLAG_UPDATE; }
    /// sets range, use -1.0 for infinite
    void range(float f) { m_bndSphere.radius(f); }
    /// returns range, -1.0 means infinite
    float range() const { return m_bndSphere.radius(); }

    /// returns light ambient color
	const vec4f & ambient() const { return m_amb; }
    /// sets light ambient color
	void ambient(const vec4f & color) { m_amb=color; m_flags|=FLAG_UPDATE; }
    /// returns light diffuse color
	const vec4f & diffuse() const { return m_dif; }
    /// sets light diffuse color
	void diffuse(const vec4f & color) { m_dif=color; m_flags|=FLAG_UPDATE; }
    /// returns light specular color
	const vec4f & specular() const { return m_spc; }
    /// sets light specular color
	void specular(const vec4f & color) { m_spc=color; m_flags|=FLAG_UPDATE; }
    /// returns light shadow color
	const vec4f & shadow() const { return m_shd; }
    /// sets light shadow color
	void shadow(const vec4f & color) { m_shd=color; }
    /// returns object as xml statement
    virtual Xml xml() const;
protected:
	/// stores light position
	vec4f m_pos;
    /// stores ambient color
    vec4f m_amb;
    /// stores diffuse color
    vec4f m_dif;
    /// stores specular color
    vec4f m_spc;
    /// stores shadow color
    vec4f m_shd;
};

//--- class proTransform --------------------------------------------
/// generic class for organizing nodes in a tree structure and handling transforms
class proTransform : public proNode {
public:
    /// default constructor
    proTransform(const std::string & name="") : proNode(name), m_isIdentity(true) { }
    /// copy constructor
    proTransform(const proTransform & source);
    /// constructor interpreting an X3D defined Transform/Group/Scene node.
    proTransform(const Xml & xs);
    /// destructor
    virtual ~proTransform() { mv_node.clear(); }
    /// returns a pointer to a physical copy of the object
    virtual proNode * copy() const { return new proTransform(*this); }

    /// culls and draws object in an efficient manner according to the provided camera context
    /** \param camera current camera settings*/
    virtual void draw(proCamera & camera);
    /// performs OpenGL initializations, calls initGraphics() of all subnodes
    virtual void initGraphics() {
        for(std::vector<proNode*>::iterator i=mv_node.begin(); i!=mv_node.end(); ++i) (*i)->initGraphics(); proNode::initGraphics(); }
    /// cleans up OpenGL initializations, calls closeGraphics() of all subnodes
    virtual void closeGraphics() {
        proNode::closeGraphics(); for(std::vector<proNode*>::iterator i=mv_node.begin(); i!=mv_node.end(); ++i) (*i)->closeGraphics(); }
    /// returns the node type
    virtual std::string type() const { return TYPE; }
	/// type name
	static const char* const TYPE;

    /// adds a direct subordinate node, optionally creates a physical copy of node and all subnodes
    virtual proNode* append(proNode* node, bool doCopy=true) { 
        if(!node) return 0; mv_node.push_back(doCopy ? node->copy() : node);  return mv_node.back(); }
    /// creates a new subordinate transform node
    virtual proTransform * create(const std::string & name="") {
        mv_node.push_back(new proTransform(name)); return static_cast<proTransform*>(mv_node.back()); }
    /// removes and optionally deletes a direct subordinate node
    virtual bool erase(proNode* node, bool doDelete=true);
    /// returns number of direct subnodes
    virtual size_t size() const { return mv_node.size(); }
    /// allows access to subnode number n
    /** Warning, for efficiency reasons no range check is performed! */
    virtual proNode * operator[](size_t n) const { return mv_node[n]; }
    /// clears all subnodes
    void clear();
    /// allows iteration of proNode graphs
    virtual proNode * next(proNode::iterator & iter);

    /// computes the bounding sphere, not for realtime!
    virtual void calcBounding(bool recursive=true);
    /// returns the bounding sphere, local transformation is already applied
    /**  a radius<0.0f is interpreted as always draw */
    virtual sphere boundingSphere() const;
    /// tests for intersection with ray
    virtual bool intersects(const line & ray) const;
    /// returns intersection point with ray or null in case of none
    virtual vec3f* intersection(const line & ray) const;
    /// returns pointer to nearest (sub-)node which is intersected by the passed ray
    virtual const proNode * query(const line & ray, unsigned int queryFlags=0xFFFFFFFF) const;

    /// sets one or more flags
    virtual void enable(unsigned int flag);
    /// unsets one or more flags
    virtual void disable(unsigned int flag);
    /// transforms this object permanently by multiplying all subnodes with matrix m
	/** for dynamic transformations, the multiply() method is preferable. */
    virtual void transform(const mat4f &);
    /// returns transformation matrix, const
    const mat4f & matrix() const { return m_mat; }
    /// sets transformation to matrix m.
    /** beware of scalings or shears! */
    void set(const mat4f & m) { m_mat=m; m_isIdentity=false; enable(FLAG_UPDATE); }
    /// sets transformation to vec6f sixdof.
    void set(const vec6f & sixdof) { m_mat=sixdof; m_isIdentity=false; enable(FLAG_UPDATE); }
    /// translates object
    void translate(const vec3f & delta) { m_mat.translate(delta); m_isIdentity=false; enable(FLAG_UPDATE); }
	/// multiplies transformation matrix with matrix m
	void multiply(const mat4f & m) { m_mat*=m; m_isIdentity=false; enable(FLAG_UPDATE); }
	/// resets transformation matrix to identity
	void reset() { m_mat=mat4f(); m_isIdentity=true; enable(FLAG_UPDATE); }
    /// returns object as xml statement
    virtual Xml xml() const;

protected:
    /// stores current transformation
    mat4f m_mat;
    /// stores whether current transformation is guaranteed an identity matrix
    bool m_isIdentity;
    /// vector for pointers to subordinate proNodes
    std::vector<proNode*> mv_node;
};

//--- class proScene ------------------------------------------------
/// a class performing global scene management and acting as a root node
class proScene : public proTransform {
public:
    /// default constructor
    proScene(const std::string & name="") : proTransform(name) { m_flags|=FLAG_RENDER; m_queryFlags=0xFFFFFFFF; }
    /// copy constructor
    proScene(const proScene & source) : proTransform(source) { }
    /// copy constructor
    proScene(const proTransform & source) : proTransform(source) { }
    /// constructor interpreting an X3D defined Transform/Group/Scene node.
    proScene(const Xml & xs) : proTransform(xs) { m_flags|=FLAG_RENDER; }

    /// performs a single render pass according to the provided camera and context by calling the associated Renderable object
    /** \param camera current camera settings*/
    virtual void draw(proCamera & camera) { proNode::draw(camera); }
    /// returns pointer to nearest (sub-)node which is intersected by the passed ray
	/** \param ray infinite ray
	 \param queryFlags (optional, default all) When performing a scene query, an object is included or excluded depending on bitwise matches between its query flags and the query's query flags.  */
    virtual const proNode * query(const line & ray, unsigned int queryFlags=0xFFFFFFFF) const { 
		const proNode* ret = proTransform::query(ray, queryFlags); return (ret==this) ? 0 : ret; }

    /// returns the node type
    virtual std::string type() const { return TYPE; }
	/// type name
	static const char* const TYPE;
};

//--- class proMesh -----------------------------------------------

/// a generic mesh geometry class
class proMesh : public proNode {
public:
	// shadow volume structure
    /// an auxiliary struct holding edge information for shadow volume generation
    struct edge {
        /// constructor initializing members
        edge(unsigned int vIndex0, unsigned int vIndex1, unsigned int nIndex0, unsigned int nIndex1) {
            vertexIndex[0]=vIndex0; vertexIndex[1]=vIndex1; normalIndex[0]=nIndex0; normalIndex[1]=nIndex1; }
        /// holds vertex indices
        unsigned int vertexIndex[2];
        /// holds normal indices
        unsigned int normalIndex[2];
    };
	
    /// default constructor, empty mesh.
    proMesh(const std::string & name="");
    /// copy constructor
    proMesh(const proMesh & source);
    /// constructor interpreting an X3D defined IndexedFaceSet node.
    proMesh(const Xml & xs);
    /// destructor
    virtual ~proMesh() { }
    /// returns a pointer to a physical copy of the object
    virtual proNode * copy() const { return new proMesh(*this); }

    /// performs a single render pass according to the provided camera and context by calling the associated Renderable object
    /** \param camera current camera settings*/
    virtual void draw(proCamera & camera);
    /// initializes GL, uploads textures to OpenGL.
    virtual void initGraphics();
    /// computes the bounding geometry, not for realtime!
    virtual void calcBounding(bool recursive=false);
    /// returns the node type
    virtual std::string type() const { return TYPE; }
	/// type name
	static const char* const TYPE;

    /// transforms this object by multiplying it with matrix m, not for realtime!.
    virtual void transform(const mat4f & m);

    /// returns material
    const proMaterial & material() const { return m_mat; }
    /// sets material
    void material(const proMaterial & mat ) { 
        m_mat=MaterialMgr::singleton()[MaterialMgr::singleton().add(mat)]; }

    /// allows direct access to coordinate data.
    std::vector<vec3f> & coords() { return mv_coord; }
    /// allows direct reading of coordinate data.
    const std::vector<vec3f> & coords() const { return mv_coord; }
    /// allows direct access to vertex normals.
    std::vector<vec3f> & vNormals() { return mv_normal; }
    /// allows direct reading of vertex normals.
    const std::vector<vec3f> & vNormals() const { return mv_normal; }
    /// allows direct access to face normals.
    std::vector<vec3f> & fNormals() { return mv_fNormal; }
    /// allows direct reading of face normals.
    const std::vector<vec3f> & fNormals() const { return mv_fNormal; }
    /// allows direct access to texture coordinate data.
    std::vector<vec2f> & texCoords() { return mv_texCoord; }
    /// allows direct reading of texture coordinate data.
    const std::vector<vec2f> & texCoords() const { return mv_texCoord; }
    /// allows direct reading of vertex colors.
    const std::vector<vec3f> & vertexColors() const { return mv_color; }
    /// allows direct access to vertex colors.
    std::vector<vec3f> & vertexColors() { return mv_color; }
    /// allows direct access to indices.
    std::vector<unsigned int> & indices() { return mv_index; }
    /// allows direct reading of indices.
    const std::vector<unsigned int> & indices() const { return mv_index; }
	
	/// allows direct reading of edges
    const std::vector<proMesh::edge> & edges() const { return mv_edge; }
    /// allows direct reading of shadow volume quads
    const std::vector<vec3f> & shadows() const { return mv_shadow; }
    /// allows direct reading of shadow volume caps
    const std::vector<vec3f> & caps() const { return mv_cap; }
    
    /// adds an individual vertex
    void addVertex(const vec3f & vtx) { mv_coord.push_back(vtx); }
    /// adds an individual vertex
    void addVertex(float x, float y, float z=0.0f) { mv_coord.push_back(vec3f(x,y,z)); }
    /// adds an individual texture coordinate
    void addTexCoord(const vec2f & uv) { mv_texCoord.push_back(uv); }
    /// adds an individual texture coordinate
    void addTexCoord(float u, float v) { mv_texCoord.push_back(vec2f(u,v)); }
    /// adds an individual normal
    void addNormal(const vec3f & vtx) { mv_normal.push_back(vtx); }
    /// adds an individual normal
    void addNormal(float x, float y, float z) { mv_normal.push_back(vec3f(x,y,z)); }
    /// adds a triangular face by specifying the vertex indices
    void addFace(unsigned int idx0, unsigned int idx1, unsigned int idx2) { 
        mv_index.push_back(idx0); mv_index.push_back(idx1); mv_index.push_back(idx2); }
    /// adds a quad face by specifying the vertex indices
    /** internally the quad is stored as 2 triangles */
    void addFace(unsigned int idx0, unsigned int idx1, unsigned int idx2, unsigned int idx3) { 
        mv_index.push_back(idx0); mv_index.push_back(idx1); mv_index.push_back(idx2);
        mv_index.push_back(idx0); mv_index.push_back(idx2); mv_index.push_back(idx3); }
    /// adds a triangular face by specifying its vertices
    void addFace(const vec3f & vtx0, const vec3f & vtx1, const vec3f & vtx2);
    /// adds a quad face by specifying its vertices
    void addFace(const vec3f & vtx0, const vec3f & vtx1, const vec3f & vtx2, const vec3f & vtx3) {
	    addFace(vtx0,vtx1,vtx2); addFace(vtx0,vtx2,vtx3); }
		
    /// returns object as xml statement
    virtual Xml xml() const;
    /// tests for intersection with ray
    virtual bool intersects(const line & ray) const;
    /// calculates the intersection point between the provided ray and this vertex array mesh
    virtual vec3f* intersection(const line & ray) const;
    /// returns pointer to this node in case it is intersected by the passed ray
    virtual const proNode * query(const line & ray, unsigned int queryFlags=0xFFFFFFFF) const { 
		return ((queryFlags&m_queryFlags) && intersects(ray)) ? this : 0; }
        
    /// returns kind of stored data
    unsigned int kind() const { return m_kind; }
    /// sets kind of stored data
    void kind(unsigned int k) { m_kind=k; }
    /// symbolic names for kind of stored data
    enum { KIND_INDEXED_TRIANGLES=0, KIND_INDEXED_LINESTRIPS };
    /// builds edge list
	/** Note that a correct edge list and shadow volume requires a well-formed closed solid object geometry as basis.*/
    bool buildEdgeList();
protected:   
    /// stores kind of stored data
    unsigned int m_kind;
    /// stores coordinates
    std::vector<vec3f> mv_coord;
    /// stores texture coords
    std::vector<vec2f> mv_texCoord;
    /// stores color values, if color per vertex
    std::vector<vec3f> mv_color;
    /// stores per vertex normals
    std::vector<vec3f> mv_normal;
    /// stores per face normals
    std::vector<vec3f> mv_fNormal;
    /// stores coordinate indices
    std::vector<unsigned int> mv_index;

    /// stores edges
    std::vector<proMesh::edge> mv_edge;
    /// caches shadow volume quads
    std::vector<vec3f> mv_shadow;
    /// caches shadow volume caps
    std::vector<vec3f> mv_cap;

	/// material data
    proMaterial m_mat;
};

#endif // _PRO_SCENE_H
