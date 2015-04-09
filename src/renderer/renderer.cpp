#include "renderer.hpp"
#include <glm/glm.hpp>
//#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glm/gtc/type_ptr.hpp>

GLuint vao;
const Mesh* mesh;

const char* vertex_file_name = "src/shaders/default.vert";
const char* fragment_file_name1 = "src/shaders/default.frag";
const char* fragment_file_name2 = "src/shaders/depth.frag";
const GLsizeiptr vertex_size = 8*sizeof((float)(1.0));
const GLsizeiptr triangle_size = 3*sizeof((unsigned int)(1));

GLuint vs_handler;
GLuint fs_handler1;
GLuint fs_handler2;
GLuint prog1;
GLuint prog2;
GLuint buffers[4];
GLuint rectBuffers[2];
GLuint fboA;
GLuint fboB;
GLuint depthTex;
GLuint colorTex;
GLuint w;
GLuint h;

bool rectb = false;

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

GLuint prepareFBO(GLuint color, GLuint depth)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
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

glm::mat4 get_mvp(const Scene& scene, const Camera& camera)
{
    glm::mat4 model = scene.get_model_matrix(0);
    //print_matrix(model);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 proj = camera.getProjectionMatrix();
    return proj*(view*model);
}

bool Renderer::initialize( const Camera& camera, const Scene& scene )
{
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);
    
    w = m_viewport[2];
    h = m_viewport[3];
    
    mesh = scene.models[0].mesh;
    
    size_t num_vertices = mesh->num_vertices();
    std::cout << mesh->num_triangles() << std::endl;
    
    vs_handler = initShader(vertex_file_name, GL_VERTEX_SHADER);
    fs_handler1 = initShader(fragment_file_name1, GL_FRAGMENT_SHADER);
    fs_handler2 = initShader(fragment_file_name2, GL_FRAGMENT_SHADER);
    
    colorTex = createRGBATexture();
    depthTex = createDepthTexture();
    
    fboA = prepareFBO(colorTex, depthTex);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // create the VAO
    glGenVertexArrays(1, &vao); // make one VAO name and give it to vao
    glBindVertexArray(vao); // from now on the VAO is vao
    
    // creat buffers for our vertex data
    glGenBuffers(4, buffers);
    
    //vertex coordinates buffer
    print_errors("before vertexPosition block");
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]); // the array buffer from now on is buffers[0]
    print_errors("after glBindBuffer for vertexPosition block");
    glBufferData(GL_ARRAY_BUFFER, vertex_size*num_vertices, mesh->get_vertices(), GL_STATIC_DRAW);
    print_errors("after glBufferData for vertexPosition block");
    glEnableVertexAttribArray(0);
    print_errors("after glEnableVertexAttribArray for vertexPosition block");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);
    print_errors("after glVertexAttribPointer for vertexPosition block");
    
    /*
    GLuint norm_id = glGetAttribLocation(prog, "vertexNormal");
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    print_errors("after glBindBuffer");
    glBufferData(GL_ARRAY_BUFFER, vertex_size*num_vertices, mesh->get_vertices(), GL_STATIC_DRAW);
    print_errors("after glBufferData");
    glEnableVertexAttribArray(norm_id);
    print_errors("after glEnableVertexAttribArray");
    glVertexAttribPointer(norm_id, 3, GL_FLOAT, GL_TRUE, vertex_size, (void*)3);
    print_errors("after vertexNormal block");
    
    GLuint tex_id = glGetAttribLocation(prog, "vertexTexCoord");
    glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size*num_vertices, mesh->get_vertices(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(tex_id);
    glVertexAttribPointer(tex_id, 2, GL_FLOAT, GL_FALSE, vertex_size, (void*)6);
    print_errors("end of vertexTexCoord block");
    */
    
    // unbind the VAO
//    glBindVertexArray(0);
    
    prog1 = glCreateProgram();
    
    glAttachShader(prog1, vs_handler);
    glAttachShader(prog1, fs_handler1);
    
    glBindAttribLocation(prog1, 0, "vertexPosition");
    
    glLinkProgram(prog1);
    
    GLint isLinked = 0;
    glGetProgramiv(prog1, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(prog1, GL_INFO_LOG_LENGTH, &maxLength);
        
        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(prog1, maxLength, &maxLength, &infoLog[0]);
        
        //The program is useless now. So delete it.
        glDeleteProgram(prog1);
        
        for (int i = 0; i < maxLength; i++)
        {
            std::cout << infoLog[i];
        }
        std::cout << std::endl;
        return false;
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_size*mesh->num_triangles(), mesh->get_triangles(), GL_STATIC_DRAW);
    print_errors("end of element block");
    
    glGenBuffers(2, rectBuffers);
    
    glBindBuffer(GL_ARRAY_BUFFER, rectBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(float), rect, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectBuffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*sizeof(unsigned int), rectIndices, GL_STATIC_DRAW);
    
    prog2 = glCreateProgram();
    
    glAttachShader(prog2, vs_handler);
    glAttachShader(prog2, fs_handler2);
    
    glBindAttribLocation(prog2, 0, "vertexPosition");
    
    glLinkProgram(prog2);
    
    isLinked = 0;
    glGetProgramiv(prog2, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(prog2, GL_INFO_LOG_LENGTH, &maxLength);
        
        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(prog2, maxLength, &maxLength, &infoLog[0]);
        
        //The program is useless now. So delete it.
        glDeleteProgram(prog2);
        
        for (int i = 0; i < maxLength; i++)
        {
            std::cout << infoLog[i];
        }
        std::cout << std::endl;
        return false;
    }
    
    print_errors("end of initialize");
    
	return true;
}

void drawVAO(const Camera& camera, const Scene& scene, GLuint prog)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
    
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_size*mesh->num_triangles(), mesh->get_triangles(), GL_STATIC_DRAW);
    
    glm::mat4 modelViewProjectionMatrix = get_mvp(scene, camera);
    
    GLuint mvp = glGetUniformLocation(prog, "mvp");
    print_errors("after glGetUniformLocation");
    glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
    print_errors("after glUniformMatrix4fv");
    
    glDrawElements(GL_TRIANGLES, mesh->num_triangles()*3, GL_UNSIGNED_INT, 0);
    print_errors("after drawElements");
}

void drawRect(const Camera& camera, const Scene& scene, GLuint prog)
{
    glBindBuffer(GL_ARRAY_BUFFER, rectBuffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectBuffers[1]);
    
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_size*mesh->num_triangles(), mesh->get_triangles(), GL_STATIC_DRAW);
    
    glm::mat4 modelViewProjectionMatrix = get_mvp(scene, camera);
    modelViewProjectionMatrix = glm::mat4(1.0);
    
    GLuint mvp = glGetUniformLocation(prog, "mvp");
    print_errors("after glGetUniformLocation");
    glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
    print_errors("after glUniformMatrix4fv");
    
    glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_INT, 0);
    print_errors("after drawElements");
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
    // sets textures of fbo to output textures
    glBindFramebuffer(GL_FRAMEBUFFER, fboA);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    print_errors("after glBindFrameBuffer(fboA)");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    //glUseProgram(prog2);
    //drawRect(camera, scene, prog2);
    
    glUseProgram(prog1);
    drawVAO(camera, scene, prog1);
    
    glDisable(GL_DEPTH_TEST);
    print_errors("after glDisable(GL_DEPTH_TEST)");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glUseProgram(prog2);
    print_errors("after glUseProgram(prog2)");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    print_errors("after glBindFramebuffer(0)");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    print_errors("after glClear");
    
    glActiveTexture(GL_TEXTURE0);
    print_errors("after glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, depthTex);
    print_errors("after glBindTexture");
    
    float wf = w*1.0;
    float hf = h*1.0;
    
    glUniform1i(glGetUniformLocation(prog2, "depth"), 0);
    glUniform1f(glGetUniformLocation(prog2, "screenWidth"), wf);
    glUniform1f(glGetUniformLocation(prog2, "screenHeight"), hf);
    print_errors("after glUniform");
    
    drawRect(camera, scene, prog2);
    print_errors("after drawVAO 2");
    
    glUseProgram(0);
    
}

void Renderer::release()
{
    glDetachShader(prog1, vs_handler);
    glDetachShader(prog1, fs_handler1);
    glDetachShader(prog2, vs_handler);
    glDetachShader(prog2, fs_handler2);
    
    glDeleteShader(vs_handler);
    glDeleteShader(fs_handler1);
    glDeleteProgram(prog1);
    glDeleteShader(fs_handler2);
    glDeleteProgram(prog2);
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
