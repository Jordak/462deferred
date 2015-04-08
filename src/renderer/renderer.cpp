#include "renderer.hpp"
#include <glm/glm.hpp>
//#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glm/gtc/type_ptr.hpp>

const char* vs_passthrough_file = "src/shaders/passthrough.vert";
const char* vs_standard_file = "src/shaders/standard.vert";
const char* fs_geometry_file = "src/shaders/geometry.frag";
const char* fs_depth_file = "src/shaders/depth.frag";
const char* fs_second_pass_file = "src/shaders/second_pass.frag";
const GLsizeiptr vertex_size = 8*sizeof((float)(1.0));
const GLsizeiptr triangle_size = 3*sizeof((unsigned int)(1));

GLuint vs_passthrough_handler;
GLuint vs_standard_handler;
GLuint fs_geometry_handler;
GLuint fs_depth_handler;
GLuint fs_second_pass_handler;
GLuint fboA;
GLuint fboB;
GLuint diffuseTex;
GLuint normalTex;
GLuint specularTex;
GLuint positionTex;
GLuint ambientTex;
//GLuint diffuseTexTex;
//GLuint ambientTexTex;
GLuint depthTex;
GLuint w;
GLuint h;
std::vector<Renderer::Renderable> renderables;
Renderer::Renderable rectRenderable;

GLfloat rect[12] = {-1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, 1.0, 0.0};
GLuint rectIndices[6] = {0, 1, 2, 0, 2, 3};

char* textFileRead(const char* fn);
void print_errors(std::string message);

GLuint createDepthTexture()
{
    GLuint tex;
    
    glGenTextures(1, &tex);
    
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 w,
                 h,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 NULL);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return tex;
}

GLuint createRGBTexture()
{
    GLuint tex;
    
    glGenTextures(1, &tex);
    
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 w,
                 h,
                 0, GL_RGB, GL_UNSIGNED_BYTE,
                 NULL);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return tex;
}

GLuint createRGBATexture()
{
    GLuint tex;
    
    glGenTextures(1, &tex);
    
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 w,
                 h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 NULL);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return tex;
}

GLuint prepareFBO()
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuseTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, specularTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, positionTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, ambientTex, 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, diffuseTexTex, 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, ambientTexTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    
    GLenum e = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch (e) {
            
        case GL_FRAMEBUFFER_UNDEFINED:
            printf("FBO Undefined\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
            printf("FBO Incomplete Attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT :
            printf("FBO Missing Attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER :
            printf("FBO Incomplete Draw Buffer\n");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED :
            printf("FBO Unsupported\n");
            break;
        case GL_FRAMEBUFFER_COMPLETE:
            printf("FBO OK\n");
            break;
        default:
            printf("FBO Problem?\n");
    }
    
    if (e != GL_FRAMEBUFFER_COMPLETE)
    {
        return(0);
    }
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
    print_errors("end prepare fbo");
    
    return fbo;
}

GLuint initShader(const char* file_name, GLenum shader_type)
{
    GLuint handler = glCreateShader(shader_type);
    char* src = textFileRead(file_name);
    const char* source = src;
    glShaderSource(handler, 1, &source, nullptr);
    free(src);
    glCompileShader(handler);
    
    print_errors("after glCompileShader");
    
    GLint isCompiled = 0;
    glGetShaderiv(handler, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(handler, GL_INFO_LOG_LENGTH, &maxLength);
        
        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(handler, maxLength, &maxLength, &errorLog[0]);
        
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(handler); // Don't leak the shader.
        
        std::cout << file_name << std::endl;
        for (int i = 0; i < maxLength; i++)
        {
            std::cout << errorLog[i];
        }
        std::cout << std::endl;
        
    }
    
    return handler;
}

void print_matrix(glm::mat4 matrix)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            std::cout << matrix[i][j] << "   ";
        }
        std::cout << std::endl;
    }
    std::cout << "\n" << std::endl;
}

glm::mat4 get_vp(const Camera& camera, const Scene& scene)
{
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 proj = camera.getProjectionMatrix();
    return proj*view;
}

GLuint prepareVAO(const float* vertices, unsigned int num_vertices, unsigned int vert_size, unsigned int stride, const unsigned int* indices, unsigned int num_triangles, GLuint* buffers)
{
    print_errors("begin prepareVAO");
    
    // create the VAO
    GLuint vao = 0;
    std::cout << "HERE" << std::endl;
    print_errors("before glGenVertexArray");
    glGenVertexArrays(1, &vao); // make one VAO name and give it to vao
    print_errors("after glGenVertexArray");
    std::cout << "HERE2" << std::endl;
    glBindVertexArray(vao); // from now on the VAO is vao
    print_errors("after glBindVertexArray");
    std::cout << "HERE3" << std::endl;
    
    // creat buffers for our vertex data
    glGenBuffers(4, buffers);
    
    
    print_errors("before vertexPosition block");
    
    //vertex coordinates buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]); // the array buffer from now on is buffers[0]
    glBufferData(GL_ARRAY_BUFFER, vert_size*num_vertices, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]); // the array buffer from now on is buffers[0]
    glBufferData(GL_ARRAY_BUFFER, vert_size*num_vertices, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, stride, (void*)(3*sizeof(float)));
    print_errors("after vertexNormal block");
    
    
    glBindBuffer(GL_ARRAY_BUFFER, buffers[2]); // the array buffer from now on is buffers[0]
    glBufferData(GL_ARRAY_BUFFER, vert_size*num_vertices, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));
    print_errors("after vertexTexCoord block");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*num_triangles*sizeof(unsigned int), indices, GL_STATIC_DRAW);
    print_errors("end of element block");
    
    // unbind the VAO
    glBindVertexArray(0);
    std::cout << "END" << std::endl;
    return vao;
}

Renderer::Renderable prepareRenderable(glm::mat4 mat, const GLfloat* vertices, const GLuint* indices,
                                       unsigned int num_vertices, unsigned int num_triangles, GLuint vertex_size,
                                       GLuint stride, GLuint vs_handler, GLuint fs_handler)
{
    print_errors("begin prepareRenderable");
    
    Renderer::Renderable renderable;
    
    renderable.transform_matrix = mat;
    renderable.num_triangles = num_triangles;
    renderable.num_vertices = num_vertices;
    
    renderable.vao = prepareVAO(vertices,
                                renderable.num_vertices,
                                vertex_size,
                                stride,
                                indices,
                                renderable.num_triangles,
                                renderable.vbo);
    
    glBindVertexArray(renderable.vao);
    
    renderable.prog = glCreateProgram();
    
    glAttachShader(renderable.prog, vs_handler);
    glAttachShader(renderable.prog, fs_handler);
    
    glBindAttribLocation(renderable.prog, 0, "vertexPosition");
    glBindAttribLocation(renderable.prog, 1, "vertexNormal");
    glBindAttribLocation(renderable.prog, 2, "vertexTexCoord");
    
    glLinkProgram(renderable.prog);
    
    GLint isLinked = 0;
    glGetProgramiv(renderable.prog, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(renderable.prog, GL_INFO_LOG_LENGTH, &maxLength);
        
        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(renderable.prog, maxLength, &maxLength, &infoLog[0]);
        
        //The program is useless now. So delete it.
        glDeleteProgram(renderable.prog);
        
        for (int i = 0; i < maxLength; i++)
        {
            std::cout << infoLog[i];
        }
        std::cout << std::endl;
    }
    glUseProgram(0);
    glBindVertexArray(0);
    std::cout << "END PREP RENDERABLE" << std::endl;
    return renderable;
}

Renderer::Renderable prepareRenderable(Scene::StaticModel model)
{
    Renderer::Renderable renderable = prepareRenderable(model.model_matrix,
                                                        &(model.mesh->get_vertices()[0].position.x),
                                                        &(model.mesh->get_triangles()[0].vertices[0]),
                                                        model.mesh->num_vertices(),
                                                        model.mesh->num_triangles(),
                                                        sizeof(MeshVertex),
                                                        sizeof(MeshVertex),
                                                        vs_standard_handler,
                                                        fs_geometry_handler);
    
    std::cout << "ARGLE" << std::endl;
    renderable.material = model.model->get_material(model.mesh->get_materialID());
    if (!renderable.material)
    {
        std::cout << "BLOP" << std::endl;
    }
    if (renderable.material)
    {
        std::cout << "BLAAAP"  << std::endl;
    }
    std::cout << "BARGLE" << std::endl;
    renderable.model = model.model;
    std::cout << "BLAOW" << std::endl;/*
    if (renderable.material->map_Kd >= 0)
    {
        
        std::cout << "BLAOW0" << std::endl;
        sf::Texture diffuseTexture = sf::Texture();
        std::cout << "BLAOW0.5" << std::endl;
        const sf::Image* diffuseImage = model.model->get_texture(renderable.material->map_Kd);
        
        std::cout << "BLAOW1" << std::endl;
        diffuseTexture.loadFromImage(*diffuseImage);
        std::cout << "BLAOW2" << std::endl;
        renderable.diffuseTexture = diffuseTexture;
        std::cout << "BLAOW3" << std::endl;
    }
    
    if (renderable.material->map_Ka >= 0)
    {
        std::cout << "BLAOW4.0.1" << std::endl;
        sf::Texture ambientTexture = sf::Texture();
        std::cout << "BLAOW4.0.2" << std::endl;
        const sf::Image* ambientImage = model.model->get_texture(renderable.material->map_Ka);
        std::cout << "BLAOW4" << std::endl;
        ambientTexture.loadFromImage(*ambientImage);
        std::cout << "BLAOW5" << std::endl;
        renderable.ambientTexture = ambientTexture;
        std::cout << "BLAOW6" << std::endl;
    }*/
    std::cout << "END PREP RENDERABLE2" << std::endl;
        
    return renderable;
}

bool Renderer::initialize( const Camera& camera, const Scene& scene )
{
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);
    
    w = m_viewport[2];
    h = m_viewport[3];
    
    vs_passthrough_handler = initShader(vs_passthrough_file, GL_VERTEX_SHADER);
    vs_standard_handler = initShader(vs_standard_file, GL_VERTEX_SHADER);
    fs_geometry_handler = initShader(fs_geometry_file, GL_FRAGMENT_SHADER);
    fs_depth_handler = initShader(fs_depth_file, GL_FRAGMENT_SHADER);
    fs_second_pass_handler = initShader(fs_second_pass_file, GL_FRAGMENT_SHADER);
    
    print_errors("after init shaders");
    
    diffuseTex = createRGBATexture();
    ambientTex = createRGBTexture();
    specularTex = createRGBTexture();
    normalTex = createRGBTexture();
    positionTex = createRGBTexture();
    depthTex = createDepthTexture();
    
    print_errors("after create textures");
    
    fboA = prepareFBO();
    
    print_errors("after prepare FBO");
    
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    std::cout << "before loop" << std::endl;
    for (unsigned int i = 0; i < scene.models.size(); i++)
    {
        std::cout << "in loop" << std::endl;
        renderables.push_back(prepareRenderable(scene.models[i]));
        print_errors("after renderables.push_back");
    }
    std::cout << "after loop" << std::endl;
    
    rectRenderable = prepareRenderable(glm::mat4(1.0),
                                       &(rect[0]),
                                       &(rectIndices[0]),
                                       4,
                                       2,
                                       3*sizeof(float),
                                       0,
                                       vs_passthrough_handler,
                                       fs_second_pass_handler);
    
    //glActiveTexture(GL_TEXTURE0);
    //print_errors("after glActiveTexture");
    //glBindTexture(GL_TEXTURE_2D, depthTex);
    //print_errors("after glBindTexture");
    
    
    //glUseProgram(rectRenderable.prog);
    
    //glUniform1i(glGetUniformLocation(rectRenderable.prog, "depth"), 0);
    
    std::cout << "AFTER PREPARE RECT" << std::endl;
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTex);
    
    std::cout << "AFTER BIND DIFFUSETEX" << std::endl;
    
    glUseProgram(rectRenderable.prog);
    
    float wf = w*1.0;
    float hf = h*1.0;
    
    print_errors("before glUniform");
    glUniform1i(glGetUniformLocation(rectRenderable.prog, "diffuse"), 0);
    glUniform1f(glGetUniformLocation(rectRenderable.prog, "screenWidth"), wf);
    glUniform1f(glGetUniformLocation(rectRenderable.prog, "screenHeight"), hf);
    print_errors("after glUniform");
    
    glUseProgram(0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    
    std::cout << "END INITIALIZE" << std::endl;
    
	return true;
}

void drawRenderable(const Camera& camera, const Scene& scene, Renderer::Renderable renderable)
{
    glUseProgram(renderable.prog);
    
    glBindVertexArray(renderable.vao);
    print_errors("after glBindVertexArray");
        
    glDrawElements(GL_TRIANGLES, renderable.num_triangles*3, GL_UNSIGNED_INT, 0);
    print_errors("after drawElements");
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
    // sets textures of fbo to output textures
    glBindFramebuffer(GL_FRAMEBUFFER, fboA);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    print_errors("after glBindFrameBuffer(fboA)");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    for (unsigned int i = 0; i < renderables.size(); i++)
    {
        
        std::cout << "BEGIN RENDER MESH LOOP" << std::endl;
        Renderer::Renderable renderable = renderables[i];
        
        glm::mat4 modelViewProjectionMatrix = get_vp(camera, scene) * renderable.transform_matrix;
        
        glUseProgram(renderable.prog);
        
        GLuint mvpMat = glGetUniformLocation(renderable.prog, "mvpMat");
        GLuint mMat = glGetUniformLocation(renderable.prog, "mMat");
        print_errors("after glGetUniformLocation");
        glUniformMatrix4fv(mvpMat, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
        glUniformMatrix4fv(mMat, 1, GL_FALSE, glm::value_ptr(renderable.transform_matrix));
        print_errors("after glUmiformMatrix4fv");
        
        std::cout << "BEFORE UNIFORM MATERIAL BLOCK" << std::endl;
        
        
        GLuint dif = glGetUniformLocation(renderable.prog, "diffuse");
        GLuint amb = glGetUniformLocation(renderable.prog, "ambient");
        GLuint spec = glGetUniformLocation(renderable.prog, "specular");
        glUniform4fv(dif, 1, glm::value_ptr(glm::vec4(renderable.material->Kd, 1.0)));
        glUniform3fv(amb, 1, glm::value_ptr(renderable.material->Ka));
        glUniform3fv(spec, 1, glm::value_ptr(renderable.material->Ks));
        print_errors("after das block");
        
        /*
        sf::Texture::bind(&renderable.diffuseTexture);
        glUniform1i(glGetUniformLocation(renderable.prog, "diffuseTex"), 0);
        
        sf::Texture::bind(&renderable.ambientTexture);
        glUniform1i(glGetUniformLocation(renderable.prog, "ambientTex"), 0);
        
        sf::Texture::bind(NULL);
        */
        
        
        std::cout << "BEFORE DRAW RENDERABLE" << std::endl;
        drawRenderable(camera, scene, renderable);
        std::cout << "END RENDER MESH LOOP" << std::endl;
    }
    
    glDisable(GL_DEPTH_TEST);
    print_errors("after glDisable(GL_DEPTH_TEST)");
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /*
    glUseProgram(rectRenderable.prog);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    
    drawRenderable(camera, scene, rectRenderable, false);
    print_errors("after Renderable 2");
     */
    
    glUseProgram(rectRenderable.prog);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTex);
    /*
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, specularTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ambientTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, diffuseTexTex);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, ambientTexTex);
  */
    
    drawRenderable(camera, scene, rectRenderable);
    print_errors("after draw rect");
}

void releaseRenderable(Renderer::Renderable renderable)
{
    glDetachShader(renderable.prog, vs_standard_handler);
    glDetachShader(renderable.prog, fs_geometry_handler);
    
    glDeleteShader(vs_standard_handler);
    glDeleteShader(fs_geometry_handler);
    
    glDeleteProgram(renderable.prog);
    
    glDeleteVertexArrays(1, &renderable.vao);
    glDeleteBuffers(4, renderable.vbo);
}

void Renderer::release()
{
    for (unsigned int i = 0; i < renderables.size(); i++)
    {
        releaseRenderable(renderables[i]);
    }
    
    releaseRenderable(rectRenderable);
}

char* textFileRead(const char* fn)
{
    FILE *fp;
    char* content = NULL;
    
    int count = 0;
    
    if (fn != NULL)
    {
        fp = fopen(fn,"rt");
        
        if (fp != NULL)
        {
            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);
            
            if (count > 0)
            {
                content = (char*)malloc(sizeof(char) * (count+1));
                count = fread(content, sizeof(char), count, fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    return content;
}

void print_errors(std::string message)
{
    bool print_message = false;
    GLenum err = glGetError();
    while (err != GL_NO_ERROR)
    {
        std::string error;
        print_message = true;
        
        switch(err) {
            case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        
        std::cerr << "GL_" << error.c_str() << std::endl;
        err=glGetError();
    }
    if (print_message)
    {
        std::cout << message << std::endl;
    }
}

