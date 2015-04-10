#version 330

in vec3 vertexPositionI;
in vec2 vertexTexCoordI;

uniform vec3 diffuseU;
uniform samplerCube diffuseTexU;

layout(location=0) out vec3 diffuseOut;
layout(location=3) out vec3 diffuseTexOut;

void main()
{
    diffuseOut = diffuseU;
    diffuseTexOut = texture(diffuseTexU, vec3(vertexTexCoordI, 1.0)).xyz;
}
