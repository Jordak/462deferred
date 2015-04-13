#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;
uniform mat4 normalMatrix;

out vec3 vertexPositionI;
out vec3 vertexNormalI;
out vec2 vertexTexCoordI;

void main()
{
	gl_Position = (viewProjectionMatrix * modelMatrix) * vec4(vertexPosition, 1.0f);
  vertexPositionI = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;
  vertexNormalI = (normalize(normalMatrix * vec4(vertexNormal, 0.0))).xyz;
  vertexTexCoordI = vertexTexCoord;
}
