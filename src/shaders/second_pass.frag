#version 330

uniform sampler2D diffuse;
uniform float screenWidth;
uniform float screenHeight;

out vec4 outputF;

void main()
{
    float x = gl_FragCoord.x / screenWidth;
    float y = gl_FragCoord.y / screenHeight;
    vec2 uv = vec2(x,y);
    outputF = texture(diffuse, uv);
}

