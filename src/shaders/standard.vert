#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;

out vec2 vertexTexCoordI;

void main()
{
	gl_Position = (viewProjectionMatrix * modelMatrix) * vec4(vertexPosition, 1.0f);
    vertexTexCoordI = vertexTexCoord*vertexPosition.xz;
}
