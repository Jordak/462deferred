#version 330

in vec2 vertexTexCoordI;

uniform vec3 diffuseU;
uniform sampler2D diffuseTexU;

layout(location=0) out vec3 diffuseOut;
layout(location=3) out vec3 diffuseTexOut;

void main()
{
    vec2 uv = vec2(vertexTexCoordI.x*20000.0, vertexTexCoordI.y*20000.0);
    
    diffuseOut = diffuseU;
    diffuseTexOut = texture(diffuseTexU, vertexTexCoordI).xyz;
}
