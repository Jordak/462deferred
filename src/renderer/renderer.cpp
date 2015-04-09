#include "renderer.hpp"
#include <glm/glm.hpp>
//#include <GL/glew.h>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glm/gtc/type_ptr.hpp>

GLuint vao;


const char* vs_passthrough_file = "src/shaders/passthrough.vert";
const char* vs_standard_file = "src/shaders/standard.vert";
const char* fs_geometry_file = "src/shaders/geometry.frag";
const char* fs_depth_file = "src/shaders/depth.frag";
const char* fs_dirlight_file = "src/shaders/dirlight.frag";

Renderer::GBuffer gBuffer;

GLuint vs_passthrough;
GLuint vs_standard;
GLuint fs_geometry;
GLuint fs_depth;
GLuint fs_dirlight;

std::vector<Renderer::VBO> meshVBOs;
Renderer::VBO rectVBO;

GLuint gFBO;
GLuint w;
GLuint h;

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
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

Renderer::GBuffer prepareGBuffer()
{
    Renderer::GBuffer gBuffer;
    
    gBuffer.diffuseID = createRGBTexture();
    gBuffer.ambientID = createRGBTexture();
    gBuffer.specularID = createRGBTexture();
    gBuffer.diffuseTexID = createRGBTexture();
    gBuffer.ambientTexID = createRGBTexture();
    gBuffer.depthID = createDepthTexture();
    
    return gBuffer;
}

GLuint prepare_G_FBO(Renderer::GBuffer gBuffer)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.diffuseID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.ambientID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.specularID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gBuffer.diffuseTexID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gBuffer.ambientTexID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBuffer.depthID, 0);
    
    /*
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
    */
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
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

Renderer::VBO prepareMeshVBO(const Scene& scene, unsigned int i)
{
    Renderer::VBO vbo;
    
    const Mesh* mesh = scene.models[i].mesh;
    vbo.num_triangles = mesh->num_triangles();
    
    glGenBuffers(4, vbo.buffers);
    
    //vertex coordinates buffer
    print_errors("before vertexPosition block");
    glBindBuffer(GL_ARRAY_BUFFER, vbo.buffers[0]); // the array buffer from now on is buffers[0]
    print_errors("after glBindBuffer for vertexPosition block");
    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex)*mesh->num_vertices(), mesh->get_vertices(), GL_STATIC_DRAW);
    print_errors("after glBufferData for vertexPosition block");
    glEnableVertexAttribArray(0);
    print_errors("after glEnableVertexAttribArray for vertexPosition block");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);
    print_errors("after glVertexAttribPointer for vertexPosition block");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.buffers[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshTriangle)*mesh->num_triangles(), mesh->get_triangles(), GL_STATIC_DRAW);
    print_errors("end of element block");
    
    vbo.prog = glCreateProgram();
    
    glAttachShader(vbo.prog, vs_standard);
    glAttachShader(vbo.prog, fs_geometry);
    
    glBindAttribLocation(vbo.prog, 0, "vertexPosition");
    
    glLinkProgram(vbo.prog);
    
    GLint isLinked = 0;
    glGetProgramiv(vbo.prog, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(vbo.prog, GL_INFO_LOG_LENGTH, &maxLength);
        
        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(vbo.prog, maxLength, &maxLength, &infoLog[0]);
        
        //The program is useless now. So delete it.
        glDeleteProgram(vbo.prog);
        
        for (int i = 0; i < maxLength; i++)
        {
            std::cout << infoLog[i];
        }
        std::cout << std::endl;
    }
    
    glUseProgram(vbo.prog);
    
    glUniform3fv(glGetUniformLocation(vbo.prog, "diffuseU"), 1, glm::value_ptr(mesh->diffuse));
    print_errors("after glUniform");
    
    glUseProgram(0);
    
    return vbo;
}

Renderer::VBO prepareRectVBO()
{
    Renderer::VBO vbo;
    vbo.num_triangles = 2;
    
    glGenBuffers(2, vbo.buffers);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo.buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(float), rect, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*sizeof(unsigned int), rectIndices, GL_STATIC_DRAW);
    
    vbo.prog = glCreateProgram();
    
    glAttachShader(vbo.prog, vs_passthrough);
    glAttachShader(vbo.prog, fs_dirlight);
    
    glBindAttribLocation(vbo.prog, 0, "vertexPosition");
    
    glLinkProgram(vbo.prog);
    
    GLint isLinked = 0;
    glGetProgramiv(vbo.prog, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(vbo.prog, GL_INFO_LOG_LENGTH, &maxLength);
        
        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(vbo.prog, maxLength, &maxLength, &infoLog[0]);
        
        //The program is useless now. So delete it.
        glDeleteProgram(vbo.prog);
        
        for (int i = 0; i < maxLength; i++)
        {
            std::cout << infoLog[i];
        }
        std::cout << std::endl;
    }
    
    float wf = w*1.0;
    float hf = h*1.0;
    
    glUseProgram(vbo.prog);
    
    glUniform1i(glGetUniformLocation(vbo.prog, "diffuse"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    print_errors("after glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseID);
    print_errors("after glBindTexture");
    
    glUniform1f(glGetUniformLocation(vbo.prog, "screenWidth"), wf);
    glUniform1f(glGetUniformLocation(vbo.prog, "screenHeight"), hf);
    
    /*
    glActiveTexture(GL_TEXTURE0);
    print_errors("after glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, depthTex);
    print_errors("after glBindTexture");
    
    glUniform1i(glGetUniformLocation(vbo.prog, "depth"), 0);
    print_errors("after glUniform");
     */
    
    glUseProgram(0);
    
    return vbo;
}

bool Renderer::initialize( const Camera& camera, const Scene& scene )
{
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);
    
    w = m_viewport[2];
    h = m_viewport[3];
    
    gBuffer = prepareGBuffer();
    
    gFBO = prepare_G_FBO(gBuffer);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    vs_passthrough = initShader(vs_passthrough_file, GL_VERTEX_SHADER);
    vs_standard = initShader(vs_standard_file, GL_VERTEX_SHADER);
    fs_geometry = initShader(fs_geometry_file, GL_FRAGMENT_SHADER);
    fs_depth = initShader(fs_depth_file, GL_FRAGMENT_SHADER);
    fs_dirlight = initShader(fs_dirlight_file, GL_FRAGMENT_SHADER);
    
    for (unsigned int i = 0; i < scene.models.size(); i++)
    {
        Scene::StaticModel model = scene.models[i];
        Renderer::VBO vbo = prepareMeshVBO(scene, i);
        meshVBOs.push_back(vbo);
    }
    
    rectVBO = prepareRectVBO();
    
    print_errors("end of initialize");
    
	return true;
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

void drawMeshVBO(const Camera& camera, const Scene& scene, unsigned int i)
{
    Renderer::VBO vbo = meshVBOs[i];
    
    glUseProgram(vbo.prog);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo.buffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.buffers[3]);
    
    GLuint mMat = glGetUniformLocation(vbo.prog, "modelMatrix");
    print_errors("after glGetUniformLocation");
    glUniformMatrix4fv(mMat, 1, GL_FALSE, glm::value_ptr(scene.get_model_matrix(i)));
    print_errors("after glUniformMatrix4fv");
    
    GLuint vpMat = glGetUniformLocation(vbo.prog, "viewProjectionMatrix");
    print_errors("after glGetUniformLocation");
    glUniformMatrix4fv(vpMat, 1, GL_FALSE, glm::value_ptr(get_vp(camera, scene)));
    print_errors("after glUniformMatrix4fv");
    
    glDrawElements(GL_TRIANGLES, 3*vbo.num_triangles, GL_UNSIGNED_INT, 0);
    print_errors("after drawElements mesh");
    
    glUseProgram(0);
}

void drawRectVBO()
{
    glUseProgram(rectVBO.prog);
    
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO.buffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectVBO.buffers[1]);
    
    glDrawElements(GL_TRIANGLES, rectVBO.num_triangles*3, GL_UNSIGNED_INT, 0);
    print_errors("after drawElements rect");
    
    glUseProgram(0);
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
    // sets textures of fbo to output textures
    glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
    print_errors("after glBindFrameBuffer(gBuffer)");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    for (unsigned int i = 0; i < scene.models.size(); i++)
    {
        drawMeshVBO(camera, scene, i);
    }
    
    glDisable(GL_DEPTH_TEST);
    print_errors("after glDisable(GL_DEPTH_TEST)");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    print_errors("after glClear");
    
    drawRectVBO();
}

void Renderer::release()
{
    /*
    glDetachShader(prog1, vs_handler);
    glDetachShader(prog1, fs_handler1);
    glDetachShader(prog2, vs_handler);
    glDetachShader(prog2, fs_handler2);
    
    glDeleteShader(vs_handler);
    glDeleteShader(fs_handler1);
    glDeleteProgram(prog1);
    glDeleteShader(fs_handler2);
    glDeleteProgram(prog2);
     */
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

