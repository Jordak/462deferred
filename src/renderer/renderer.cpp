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
const char* fs_cubegeo_file = "src/shaders/cubegeo.frag";
const char* fs_depth_file = "src/shaders/depth.frag";
const char* fs_dirlight_file = "src/shaders/dirlight.frag";

Scene::DirectionalLight sunlight;

Renderer::GBuffer gBuffer;

GLuint vs_passthrough;
GLuint vs_standard;
GLuint fs_geometry;
GLuint fs_cubegeo;
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

bool approx(float A, float B)
{
    float epsilon = .00000001;
    float diff = A-B;
    return (diff < epsilon) && (-diff < epsilon);
}

bool isCube(const Mesh* mesh)
{
    return false;
    if (mesh->num_vertices() != 8 || mesh->num_triangles() != 12)
    {
        return false;
    }

    float smallest = 0.0f;
    for (unsigned int i = 0; i < mesh->num_vertices(); i++)
    {
        for (unsigned int j = i+1; j < mesh->num_vertices(); j++)
        {
            glm::vec3 diff = mesh->get_vertices()[i].position - mesh->get_vertices()[j].position;
            float dist = glm::length(diff);
            if (smallest == 0.0f || dist < smallest)
            {
                smallest = dist;
            }
        }
    }

    for (unsigned int i = 0; i < mesh->num_vertices(); i++)
    {
        for (unsigned int j = i+1; j < mesh->num_vertices(); j++)
        {
            glm::vec3 diff = mesh->get_vertices()[i].position - mesh->get_vertices()[j].position;
            float dist = glm::length(diff)/smallest;
            if (!approx(dist, 1.0f) && !approx(dist, sqrt(2.0f)) && !approx(dist, sqrt(3.0f)))
            {
                return false;
            }
        }
    }

    return true;
}

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

GLuint createTextureFromImage(const sf::Image* image)
{
    if (image->getPixelsPtr() == nullptr ||
        image->getSize().x <= 0 || image->getSize().y <= 0 ||
        image->getSize().x > GL_MAX_TEXTURE_SIZE || image->getSize().y > GL_MAX_TEXTURE_SIZE)
    {
        std::cout << "empty" << std::endl;
        std::cout << image->getSize().x << ", " << image->getSize().y << std::endl;
        return 0;
    }
    
    GLuint tex;
    
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    std::cout << image->getSize().x << ", " << image->getSize().y << std::endl;
    
    print_errors("before glTexImage2D for texture");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image->getSize().x,
                 image->getSize().y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image->getPixelsPtr());
    print_errors("after glTexImage2D for texture");
    
    return tex;
}

GLuint createCubemapFromImage(const sf::Image* image)
{
    if (image->getPixelsPtr() == nullptr ||
        image->getSize().x <= 0 || image->getSize().y <= 0 ||
        image->getSize().x > GL_MAX_TEXTURE_SIZE || image->getSize().y > GL_MAX_TEXTURE_SIZE)
    {
        std::cout << "empty" << std::endl;
        return 0;
    }

    GLuint tex;

    print_errors("before gen for cube");
    glGenTextures(1, &tex);

    glActiveTexture(GL_TEXTURE0);
    print_errors("before bind for cube");
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    print_errors("before parameters for cube");
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    print_errors("before glTexImage2D for cube");
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                     image->getSize().x, image->getSize().y,
                     0, GL_RGB, GL_UNSIGNED_BYTE, image->getPixelsPtr());
    }
    print_errors("after glTexImage2D for cube");
    
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
    gBuffer.positionID = createRGBTexture();
    gBuffer.normalID = createRGBTexture();
    gBuffer.depthID = createDepthTexture();
    
    return gBuffer;
}

GLuint prepare_G_FBO(Renderer::GBuffer gBuffer)
{
    print_errors("begin prepare_G_FBO");
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    int res;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
    std::cout << res << std::endl;
    
    print_errors("before glFramebufferTexture2D");
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.diffuseID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.ambientID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.specularID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gBuffer.diffuseTexID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gBuffer.ambientTexID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gBuffer.positionID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, gBuffer.normalID, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBuffer.depthID, 0);
    print_errors("after glFramebufferTexture2D");
    
    {
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
    }
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
    print_errors("end prepare_G_FBO");

    return fbo;
}

GLuint initShader(const char* file_name, GLenum shader_type)
{
    print_errors("begin initShader");
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
    
    print_errors("end initShader");
    return handler;
}

Renderer::VBO prepareMeshVBO(const Scene& scene, unsigned int i)
{
    Renderer::VBO vbo;
    
    const Mesh* mesh = scene.models[i].mesh;
    vbo.num_triangles = mesh->num_triangles();
    
    glGenBuffers(2, vbo.buffers);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo.buffers[0]); // the array buffer from now on is buffers[0]
    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex)*mesh->num_vertices(), mesh->get_vertices(), GL_STATIC_DRAW);
    
    //vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);

    //vertex normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(MeshVertex), (void*)(sizeof(glm::vec3)));

    //vertex texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)(2*sizeof(glm::vec3)));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshTriangle)*mesh->num_triangles(), mesh->get_triangles(), GL_STATIC_DRAW);
    print_errors("end of element block");
    
    vbo.prog = glCreateProgram();
    
    glAttachShader(vbo.prog, vs_standard);
    glAttachShader(vbo.prog, fs_geometry);
    
    print_errors("before attrib binds");
    glBindAttribLocation(vbo.prog, 0, "vertexPosition");
    glBindAttribLocation(vbo.prog, 1, "vertexNormal");
    glBindAttribLocation(vbo.prog, 2, "vertexTexCoord");
    print_errors("after attrib binds");
    
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
    
    vbo.diffuse = mesh->diffuse;
    vbo.ambient = mesh->ambient;
    vbo.specular = mesh->specular;

    vbo.diffuseTex = createTextureFromImage(mesh->diffuseImg);
    vbo.ambientTex = createTextureFromImage(mesh->ambientImg);
    
    glUseProgram(vbo.prog);
    
    glUniform3fv(glGetUniformLocation(vbo.prog, "diffuseU"), 1, glm::value_ptr(vbo.diffuse));
    glUniform3fv(glGetUniformLocation(vbo.prog, "ambientU"), 1, glm::value_ptr(vbo.ambient));
    glUniform3fv(glGetUniformLocation(vbo.prog, "specularU"), 1, glm::value_ptr(vbo.specular));
    glUniform1i(glGetUniformLocation(vbo.prog, "diffuseTexU"), 0);
    glUniform1i(glGetUniformLocation(vbo.prog, "ambientTexU"), 1);
    
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
    
    glUniform1f(glGetUniformLocation(vbo.prog, "screenWidth"), wf);
    glUniform1f(glGetUniformLocation(vbo.prog, "screenHeight"), hf);

    glUniform3fv(glGetUniformLocation(vbo.prog, "lightDir"), 1, glm::value_ptr(sunlight.direction));
    glUniform3fv(glGetUniformLocation(vbo.prog, "lightColor"), 1, glm::value_ptr(sunlight.color));
    glUniform1f(glGetUniformLocation(vbo.prog, "lightAmbient"), sunlight.ambient);

    glUniform1i(glGetUniformLocation(vbo.prog, "diffuse"), 0);
    glUniform1i(glGetUniformLocation(vbo.prog, "ambient"), 1);
    glUniform1i(glGetUniformLocation(vbo.prog, "specular"), 2);
    glUniform1i(glGetUniformLocation(vbo.prog, "diffuseTex"), 3);
    glUniform1i(glGetUniformLocation(vbo.prog, "ambientTex"), 4);
    glUniform1i(glGetUniformLocation(vbo.prog, "position"), 5);
    glUniform1i(glGetUniformLocation(vbo.prog, "normal"), 6);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseID);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.ambientID);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.specularID);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseTexID);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gBuffer.ambientTexID);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gBuffer.positionID);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normalID);
    
    glUseProgram(0);
    
    return vbo;
}

bool Renderer::initialize( const Camera& camera, const Scene& scene )
{
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    sunlight = scene.get_sunlight();
    
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
    fs_cubegeo = initShader(fs_cubegeo_file, GL_FRAGMENT_SHADER);
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

void drawMeshVBO(const Camera& camera, const Scene& scene, unsigned int i)
{
    Renderer::VBO vbo = meshVBOs[i];
    
    glUseProgram(vbo.prog);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo.buffers[0]); // the array buffer from now on is buffers[0]
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(MeshVertex), (void*)(sizeof(glm::vec3)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)(2*sizeof(glm::vec3)));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.buffers[1]);
    
    glm::mat4 modelMatrix = scene.get_model_matrix(i);
    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

    GLuint mMat = glGetUniformLocation(vbo.prog, "modelMatrix");
    glUniformMatrix4fv(mMat, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLuint nMat = glGetUniformLocation(vbo.prog, "normalMatrix");
    glUniformMatrix4fv(nMat, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    
    GLuint vpMat = glGetUniformLocation(vbo.prog, "viewProjectionMatrix");
    glUniformMatrix4fv(vpMat, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vbo.diffuseTex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, vbo.ambientTex);
    
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
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseID);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.ambientID);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.specularID);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseTexID);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gBuffer.ambientTexID);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gBuffer.positionID);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normalID);
    
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
    
    GLuint attachments[7] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                             GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6};
    glDrawBuffers(7, attachments);
    
    for (unsigned int i = 0; i < scene.models.size(); i++)
    {
        drawMeshVBO(camera, scene, i);
    }
    
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

