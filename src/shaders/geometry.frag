#version 330

in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 mvpMat;
uniform mat4 mMat;

uniform vec4 diffuse;
uniform vec3 ambient;
uniform vec3 specular;
//uniform sampler2D diffuseTex;
//uniform sampler2D ambientTex;

layout (location=0) out vec4 diffuseOut;
layout (location=1) out vec3 normalOut;
layout (location=2) out vec3 specularOut;
layout (location=3) out vec3 positionOut;
layout (location=4) out vec3 ambientOut;
//layout (location=5) out vec3 diffuseTexOut;
//layout (location=6) out vec3 ambientTexOut;

void main()
{
    diffuseOut = diffuse;
    normalOut = normalize(normal);
    specularOut = specular;
    positionOut = position;
    ambientOut = ambient;
    //diffuseTexOut = texture(diffuseTex, texCoord).xyz;
    //ambientTexOut = texture(ambientTex, texCoord).xyz;
}
