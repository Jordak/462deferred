#version 330

layout(location=0) out vec3 diffuseOut;

uniform vec3 diffuseU;

void main()
{
    diffuseOut = diffuseU;
}
