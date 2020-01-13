#ifndef _PRO_RENDERER_H
#define _PRO_RENDERER_H

/** @file proRenderer.h
 \brief render backend abstraction layer and OpenGL implementation
 */
#include "proScene.h"
#include <map>
#include <string>

//--- class Renderable ---------------------------------------------

/// abstract base class for all renderable objects
class Renderable {
public:
	/// default destructor
	virtual ~Renderable() { }
    /// updates object
    virtual void update() { }
	/// draw method, interface definition
	virtual void draw(proCamera & camera)=0;
    /// pushs current matrix and multiplies it with provided matrix, for camera
    virtual void push(const mat4f &) { }
    /// pops current matrix
    virtual void pop() { }
};

//--- class Renderer -----------------------------------------------

/// a class abstracting a render backend (e.g., OpenGL, DirectX)
/** Technically a Renderer instance mainly serves as a collection and central 
  access point to a set of Renderable factory functions. */
class Renderer {
public:
	/// destructor
	virtual ~Renderer() { }
	/// initializes renderer, normally called by constructor, but might be necessary after a fullscreen switch
	virtual void init() { }
	/// tries to return a new renderable suitable for the passed node
	virtual Renderable * create(const proNode & node);
	/// registers a Renderable factory function fitting to a node class
	void registerFactory(Renderable *(*factoryFunc)(const proNode &), const std::string & type) {
		mm_RenderableFactory.insert(make_pair(type, factoryFunc)); }
protected:
	/// stores Renderable factory methods
	std::map<std::string, Renderable *(*)(const proNode &)> mm_RenderableFactory;
};

//--- class RendererGL ---------------------------------------------

/// a class providing an OpenGL render backend
class RendererGL : public Renderer {
public:
	/// constructor
	RendererGL();
	/// initializes renderer, normally called by constructor, but might be necessary after a fullscreen switch
	virtual void init();
};

//--- class RenderCameraGL -----------------------------------------

/// an OpenGL based Renderer for scene root nodes
class RenderCameraGL : public Renderable {
public:
	/// static factory method
	static Renderable* create(const proNode & node);
	/// draws renderable
	virtual void draw(proCamera & camera) { }
    /// updates object
    virtual void update();
    /// pushs current matrix and multiplies it with provided matrix
    virtual void push(const mat4f & matrix);
    /// pops current matrix and OpenGL model view matrix
    virtual void pop();
protected:
	/// constructor
	RenderCameraGL(const proCamera & cam);
	/// reference to corresponding camera
	const proCamera & m_camera;
};

//--- class RenderLightGL ------------------------------------------

/// an OpenGL based Renderer for light nodes
class RenderLightGL : public Renderable {
public:
	/// static factory method
	static Renderable* create(const proNode & node);
    /// updates object
    virtual void update();
	/// draws renderable
	virtual void draw(proCamera & camera);
protected:
	/// constructor
	RenderLightGL(const proLight & scene);
	/// reference to corresponding light node
	const proLight & m_light;
    /// stores OpenGL light id
    unsigned int m_id;
private:
    /// generates unique names for lights
    static unsigned int s_counter;
};

//--- class RenderSceneGL ------------------------------------------

/// an OpenGL based Renderer for scene root nodes
class RenderSceneGL : public Renderable {
public:
	/// static factory method
	static Renderable* create(const proNode & node);
	/// draws renderable
	virtual void draw(proCamera & camera);
protected:
	/// constructor
	RenderSceneGL(const proScene & scene) : m_scene(scene) { }
	/// reference to corresponding mesh node
	const proScene & m_scene;
};

//--- class RenderMeshGL -------------------------------------------

/// an OpenGL based Renderer for mesh nodes
class RenderMeshGL : public Renderable {
public:
	/// static factory method
	static Renderable* create(const proNode & node);
	/// draws renderable
	virtual void draw(proCamera & camera);
protected:
	/// constructor
	RenderMeshGL(const proMesh & mesh);
	/// reference to corresponding mesh node
	const proMesh & m_mesh;
};

#endif // _PRO_RENDERER_H
