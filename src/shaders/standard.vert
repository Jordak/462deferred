#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

uniform mat4 mvpMat;
uniform mat4 mMat;

out vec3 position;
out vec3 normal;
out vec2 texCoord;

void main()
{
	gl_Position = mvpMat * vec4(vertexPosition, 1.0f);
    position = (mMat * vec4(vertexPosition, 1.0)).xyz;
    normal = (mMat * vec4(vertexNormal, 0.0)).xyz;
    texCoord = vertexTexCoord;
}
