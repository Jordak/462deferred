#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <renderer/camera.hpp>
#include <scene/scene.hpp>

class Renderer {
public:

    struct Renderable
    {
        glm::mat4 transform_matrix;
        
        float* vertices;
        unsigned int* indices;
        
        unsigned int vao;
        unsigned int vbo[4];
        unsigned int prog;
        
        unsigned int num_triangles;
        unsigned int num_vertices;
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