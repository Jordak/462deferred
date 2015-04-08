#version 330

in vec3 vertexPosition;
//in vec3 vertexNormal;
//in vec2 vertexTexCoord;

uniform mat4 mvp;

out vec4 color;

void main()
{
	gl_Position = mvp * vec4(vertexPosition, 1.0f);
}
