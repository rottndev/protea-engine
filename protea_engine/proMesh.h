#ifndef _PRO_MESH_H
#define _PRO_MESH_H

/** @file proMesh.h
 \brief Contains the classes protea meshUtils and ModelMgr

 \author  gf
 $Revision: 2.8 $
 License notice (zlib license):

 (c) 2008-2009 by Gerald Franz, www.viremo.de

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

#include "proMath.h"
class proMesh;
class proNode;
class proTransform;

/// a class collecting utility functions for mesh and global scene manipulation
class meshUtils {
public:
	/// generates per face normals
	static void genFNormals(proMesh & m);
	/// generates per vertex normals based on an optional crease angle in degrees
	static void genVNormals(proMesh & m, float creaseAngle=30.0f);
	/// generates texture coordinates:
	static void genTexCoords(proMesh & m, const vec2f & texScale=vec2f(1.0f,1.0f));

	/// recursively subdivides a node and its subnodes
	static unsigned int subdivide(proNode & node, float maxDist, bool ignoreTransp=true);
	/// subdivides a mesh
	static unsigned int subdivide(proMesh & mesh, float maxDist);

	/// converts a mesh from a Z up right-handed coordinate system to a Y up right-handed coordinate system
	static void zup2yup(proMesh & m);
	/// converts a mesh from a Y up right-handed coordinate system to a Z up right-handed coordinate system
	static void yup2zup(proMesh & m);

	/// removes all transformations from a node
	/** all transformations are applied to the vertex coordinates */
	static void flattenTransforms(proNode & node);
	/// recursively flattens scene graph
	/** requires a call to flattenTransforms before */
	static void flattenHierarchy(proTransform & parent, proNode & node);
};

//--- class ModelMgr --------------------------------------------

#include <map>

/// a singleton class managing model resource loaders and savers
class ModelMgr {
public:
	/// returns singleton instance
	static ModelMgr & singleton() {
		if(!sp_instance) sp_instance=new ModelMgr; return *sp_instance; }
	/// returns pointer to singleton instance
	static ModelMgr * singletonPtr() { return &singleton(); }

	/// tries to load a file using previously registered or hardcoded loader functions
	/** \param filename path to the file to be loaded, type will be identified by suffix
	\return pointer to loaded model or 0. */
	proNode * load(const std::string & filename);
	/// tries to save a model to a file using previously registered saver functions
	/** \param model the model/node to be saved
	\param filename path to the file to be loaded, type will be identified by suffix
	\return 0 in case of success. */
	int save(const proNode & model, const std::string & filename);

	/// registers a loader function handling a file type identified by suffix
	/** The function has to be of type proNode * loadXYZ(const std::string & filename).*/
	void loaderRegister(proNode *(*loadFunc)(const std::string &), const std::string & suffix);
	/// registers a saver function handling a file type identified by suffix
	/** The function has to be of type int saveXYZ(const proNode & model, const std::string & filename).*/
	void saverRegister(int(*saveFunc)(const proNode &, const std::string &), const std::string & suffix);

	/// returns true in case a loader for this kind of model file is available
	bool loaderAvailable(const std::string & suffix) const;
	/// returns true in case a saver for this kind of model file is available
	bool saverAvailable(const std::string & suffix) const;
protected:    
	/// default constructor registering built-in loaders and savers
	ModelMgr();
	/// pointer to singleton instance
	static ModelMgr* sp_instance;

	/// built-in loader function for raw ascii triangle lists
	static proNode * loadRaw(const std::string & filename);
	/// built-in X3D file loader
	static proNode * loadX3d(const std::string & filename);
	/// built-in X3D file saver
	static int saveX3d(const proNode & model, const std::string & filename);

	/// map associating suffixes to loader functions
	std::map<std::string, proNode * (*)(const std::string &)> mm_loader;
	/// map associating suffixes to saver functions
	std::map<std::string, int (*)(const proNode &, const std::string &)> mm_saver;	
};

#endif // _PRO_MESH_H
