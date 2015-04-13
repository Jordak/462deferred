#version 330

in vec3 vertexPositionI;
in vec3 vertexNormalI;
in vec2 vertexTexCoordI;

uniform vec3 diffuseU;
uniform vec3 ambientU;
uniform vec3 specularU;
uniform sampler2D diffuseTexU;
uniform sampler2D ambientTexU;

layout(location=0) out vec3 diffuseOut;
layout(location=1) out vec3 ambientOut;
layout(location=2) out vec3 specularOut;
layout(location=3) out vec3 diffuseTexOut;
layout(location=4) out vec3 ambientTexOut;
layout(location=5) out vec3 positionOut;
layout(location=6) out vec3 normalOut;

void main()
{
    vec2 uv = vec2(vertexTexCoordI.x/2.0, vertexTexCoordI.y/2.0);
    
    diffuseOut = diffuseU;
    ambientOut = ambientU;
    specularOut = specularU;
    diffuseTexOut = texture(diffuseTexU, vertexTexCoordI).xyz;
    ambientTexOut = texture(ambientTexU, vertexTexCoordI).xyz;
    positionOut = vertexPositionI;
    normalOut = vertexNormalI;
}
