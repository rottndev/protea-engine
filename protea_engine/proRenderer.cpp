#include "proRenderer.h"

#ifdef _HAVE_GL
# ifdef _MSC_VER
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif
# include <GL/gl.h>
# include <GL/glu.h>
#endif

#include "proStr.h"

using namespace std;

//--- class Renderer -----------------------------------------------

Renderable * Renderer::create(const proNode & node) {
	map<string, Renderable *(*)(const proNode &)>::iterator it = mm_RenderableFactory.find(node.type());
	return it!=mm_RenderableFactory.end() ? (*(it->second))(node) : 0;
}

//--- class RendererGL ---------------------------------------------

RendererGL::RendererGL() : Renderer() {
	registerFactory(RenderMeshGL::create, proMesh::TYPE);
	registerFactory(RenderSceneGL::create, proScene::TYPE);
	registerFactory(RenderLightGL::create, proLight::TYPE);
	registerFactory(RenderCameraGL::create, proCamera::TYPE);
	init();
}

void RendererGL::init() {
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
}

//--- class RenderCameraGL -----------------------------------------

Renderable* RenderCameraGL::create(const proNode & node) {
	try {
		const proCamera & cam = dynamic_cast<const proCamera & >(node);
		return new RenderCameraGL(cam);
	}
	catch(bad_cast) {
		return 0;
	}
}

RenderCameraGL::RenderCameraGL(const proCamera & cam) : m_camera(cam) {
}

void RenderCameraGL::push(const mat4f & matrix) {
	glPushMatrix();
	glMultMatrixf(&matrix[0]);
}

void RenderCameraGL::pop() { 
	glPopMatrix();
}

void RenderCameraGL::update() {
    // setup the projection matrix:
    glMatrixMode( GL_PROJECTION ); 
    glLoadIdentity();
    glFrustum(m_camera.dim()[0]*m_camera.dim()[4], m_camera.dim()[1]*m_camera.dim()[4], m_camera.dim()[2]*m_camera.dim()[4], 
		m_camera.dim()[3]*m_camera.dim()[4], m_camera.dim()[4], m_camera.dim()[5]);
    // setup the modelview matrix:
    glMatrixMode( GL_MODELVIEW ); 
    glRotatef(-90, 1,0,0); // fake offset transform
    glRotatef(-m_camera.pos()[R], 0,1,0);
    glRotatef(-m_camera.pos()[P], 1,0,0);
    glRotatef(-m_camera.pos()[H], 0,0,1);
    glTranslatef(-m_camera.pos()[X], -m_camera.pos()[Y], -m_camera.pos()[Z]);
}
 

//--- class RenderLightGL ------------------------------------------

unsigned int RenderLightGL::s_counter=0;

Renderable* RenderLightGL::create(const proNode & node) {
	try {
		const proLight & light = dynamic_cast<const proLight & >(node);
		return new RenderLightGL(light);
	}
	catch(bad_cast) {
		return 0;
	}
}

RenderLightGL::RenderLightGL(const proLight & light) : m_light(light), m_id(GL_LIGHT0+(s_counter++%GL_MAX_LIGHTS)) {
}

void RenderLightGL::update() {
    glLightfv(GLenum(m_id), GL_AMBIENT, &m_light.ambient()[0]);
    glLightfv(GLenum(m_id), GL_DIFFUSE, &m_light.diffuse()[0]);
    glLightfv(GLenum(m_id), GL_SPECULAR,&m_light.specular()[0]);
    m_light.flags() & FLAG_ACTIVE ? glEnable(GLenum(m_id)) : glDisable(GLenum(m_id));
}

void RenderLightGL::draw(proCamera & camera) {
    if(!(camera.flags()&FLAG_LIGHT)) return;
    if((m_light.flags()&FLAG_SHADOW)&&(camera.flags()&FLAG_SHADOW)) {
        glStencilFunc(GL_LESS, 0x0, 0xff);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 1, 1, 0, 0, 1);
        glEnable(GL_BLEND);
        glColor4fv(&m_light.shadow()[0]);
        glRecti(0,1, 1,0);
        glDisable(GL_BLEND);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        return;
    }
    if(m_light.flags()&FLAG_UPDATE) update();
    glLightfv(GLenum(m_id), GL_POSITION,&m_light.pos()[0]);
}


//--- class RenderSceneGL ------------------------------------------

Renderable* RenderSceneGL::create(const proNode & node) {
	try {
		const proScene & scene = dynamic_cast<const proScene & >(node);
		return new RenderSceneGL(scene);
	}
	catch(bad_cast) {
		return 0;
	}
}

void RenderSceneGL::draw(proCamera & camera) {
	unsigned int flags=camera.flags();
	proScene & scene = const_cast<proScene &>(m_scene); // dirty but efficient
	if(flags&FLAG_WIREFRAME) glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );
	else if(flags&FLAG_LIGHT) {
		glEnable(GL_LIGHTING);
		camera.flags()=FLAG_LIGHT;
		scene.proTransform::draw(camera);
	}
	if(flags&FLAG_RENDER) {
		camera.flags()=FLAG_RENDER;
		scene.proTransform::draw(camera);

		camera.flags()=FLAG_TRANSPARENT;
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		// TODO sort transparent objects by z distance before rendering
		scene.proTransform::draw(camera);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	if(flags&FLAG_WIREFRAME) glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
	else if(flags&FLAG_LIGHT) glDisable(GL_LIGHTING);
		
	if((flags&FLAG_SHADOW)&&!(flags&FLAG_WIREFRAME)) {
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		// iterate through lights:
		vector<proLight*> vLight;
		for(proNode::iterator iter=&scene; iter!=0; ++iter) 
		    if((iter->type()==proLight::TYPE)&&(iter->flags()&FLAG_SHADOW)) 
			vLight.push_back(static_cast<proLight*>(*iter));
		for(size_t i=0;i<vLight.size(); ++i) {
		    if(i>0) glClear (GL_STENCIL_BUFFER_BIT);        
		    camera.light(vLight[i]);
		    // draw volumes:
		    glDisableClientState(GL_NORMAL_ARRAY);
		    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		    if(vLight.size()>1) camera.light()->flags()|=FLAG_UPDATE; // since we can store only one set of shadow volumes, multiple lights must be always updated
		    camera.flags()=FLAG_SHADOW;
		    scene.proTransform::draw(camera);

		    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		    glEnableClientState(GL_NORMAL_ARRAY);
		    
		    // tint shadow:
		    camera.flags()|=FLAG_LIGHT;
		    camera.light()->draw(camera);
		}
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
	}
	camera.flags()=flags;
}


//--- class RenderMeshGL -------------------------------------------

Renderable* RenderMeshGL::create(const proNode & node) {
	try {
		const proMesh & mesh = dynamic_cast<const proMesh & >(node);
		return new RenderMeshGL(mesh);
	}
	catch(bad_cast) {
		return 0;
	}
}

RenderMeshGL::RenderMeshGL(const proMesh & mesh) : m_mesh(mesh) {
	const_cast<proMaterial &>(m_mesh.material()).loadTexture();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
}

void RenderMeshGL::draw(proCamera & camera) {
    // normal / transparent pass:
    if(((camera.flags()&FLAG_RENDER)&&!(m_mesh.flags()&FLAG_TRANSPARENT))
		|| ((camera.flags()&FLAG_TRANSPARENT)&&(m_mesh.flags()&FLAG_TRANSPARENT)) ) { 
		// apply material:
		const proMaterial & mat = m_mesh.material();
		
        glColor4fv(&mat.color()[0]);
		if(m_mesh.flags()&FLAG_FRONT_AND_BACK) glDisable(GL_CULL_FACE);
        if(mat.texId()&&m_mesh.texCoords().size()) {
            glEnable( GL_TEXTURE_2D );
            glBindTexture( GL_TEXTURE_2D, mat.texId() );
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer  ( 2, GL_FLOAT, 0, &m_mesh.texCoords()[0] );
        }
        glVertexPointer  (3, GL_FLOAT, 0, &m_mesh.coords()[0]);
        glNormalPointer  (   GL_FLOAT, 0, &m_mesh.vNormals()[0]);
    
        if(m_mesh.vertexColors().size()) {
            glEnableClientState ( GL_COLOR_ARRAY );
            glColorPointer  ( 3, GL_FLOAT, 0, &m_mesh.vertexColors()[0] );
        }
    
        glDrawElements ( GL_TRIANGLES, m_mesh.indices().size(), GL_UNSIGNED_INT, &m_mesh.indices()[0] );
    
        if(m_mesh.vertexColors().size()) glDisableClientState ( GL_COLOR_ARRAY );
		if(m_mesh.texCoords().size()&&mat.texId()) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisable(GL_TEXTURE_2D);
		}
		if(m_mesh.flags()&FLAG_FRONT_AND_BACK) glEnable(GL_CULL_FACE);
    }
    
    // shadow volume pass:
    if((camera.flags()&FLAG_SHADOW)&&camera.light()&&(m_mesh.flags()&FLAG_SHADOW)&&m_mesh.edges().size()) {
        if((m_mesh.boundingSphere().radius()>=0.0f)&&(camera.light()->range()>=0.0f)&&(m_mesh.boundingSphere().sqrDistTo(camera.light()->pos())>camera.light()->range()*camera.light()->range()))
            return;    
        glStencilFunc(GL_ALWAYS, 0x0, 0xff);
        if(m_mesh.flags()&FLAG_ZFAIL) { // Carmack's reverse:
            // draw backs: 
            glCullFace(GL_FRONT);
            glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
            glVertexPointer  (3, GL_FLOAT, 0, &m_mesh.shadows()[0]);
            glDrawArrays ( GL_QUADS, 0, m_mesh.shadows().size());
            glVertexPointer  (3, GL_FLOAT, 0, &m_mesh.caps()[0]);
            glDrawArrays ( GL_TRIANGLES, 0, m_mesh.caps().size());
            // draw fronts:
            glCullFace(GL_BACK);
            glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
            glVertexPointer  (3, GL_FLOAT, 0, &m_mesh.shadows()[0]);
            glDrawArrays ( GL_QUADS, 0, m_mesh.shadows().size());
            glVertexPointer  (3, GL_FLOAT, 0, &m_mesh.caps()[0]);
            glDrawArrays ( GL_TRIANGLES, 0, m_mesh.caps().size());
        }
        else { // z pass:
            glVertexPointer  (3, GL_FLOAT, 0, &m_mesh.shadows()[0]);
            // draw fronts:
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
            glDrawArrays ( GL_QUADS, 0, m_mesh.shadows().size());
            // draw backs: 
            glCullFace(GL_FRONT);
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
            glDrawArrays ( GL_QUADS, 0, m_mesh.shadows().size());
            glCullFace(GL_BACK);
        }
    }
}

