#version 330

in vec3 vertexPosition;

uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;

out vec4 color;

void main()
{
	gl_Position = (viewProjectionMatrix * modelMatrix) * vec4(vertexPosition, 1.0f);
}
