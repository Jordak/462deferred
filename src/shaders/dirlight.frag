#version 330

uniform sampler2D diffuse;
uniform sampler2D diffuseTex;
uniform float screenWidth;
uniform float screenHeight;

out vec4 finalColor;

void main()
{
    float x = gl_FragCoord.x / screenWidth;
    float y = gl_FragCoord.y / screenHeight;
    vec2 uv = vec2(x,y);
    
    vec3 diffuseColor = texture(diffuse, uv).xyz;
    vec3 diffuseTexColor = texture(diffuseTex, uv).xyz;
    
    finalColor = vec4(diffuseColor*diffuseTexColor, 1.0);
}

