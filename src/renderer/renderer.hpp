#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <renderer/camera.hpp>
#include <scene/scene.hpp>
#include <SFML/OpenGL.hpp>

class Renderer {
public:

    struct VBO
    {
        GLuint buffers[2];
        GLuint prog;
        GLuint num_triangles;
        
        glm::vec3 diffuse;
        glm::vec3 ambient;
        glm::vec3 specular;
        
        GLuint diffuseTex;
        GLuint ambientTex;
    };
    
    struct GBuffer
    {
        GLuint diffuseID;
        GLuint ambientID;
        GLuint specularID;
        GLuint diffuseTexID;
        GLuint ambientTexID;
        GLuint depthID;
    };
    
	// You may want to build some scene-specific OpenGL data before the first frame
	bool initialize( const Camera& camera, const Scene& scene );

	/*
	 * Render a frame to the currently active OpenGL context.
	 * It's best to keep all your OpenGL-specific data in the renderer; keep the Scene class clean.
	 * This function should not modify the scene or camera.
	 */
	void render( const Camera& camera, const Scene& scene );

	// release all OpenGL data and allocated memory
	// you can do this in the destructor instead, but a callable function lets you swap scenes at runtime
	void release();

};

#endif // #ifndef _RENDERER_H_