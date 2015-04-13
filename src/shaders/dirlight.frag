#version 330

uniform sampler2D diffuse;
uniform sampler2D ambient;
uniform sampler2D specular;
uniform sampler2D diffuseTex;
uniform sampler2D ambientTex;
uniform sampler2D position;
uniform sampler2D normal;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float lightAmbient;

uniform float screenWidth;
uniform float screenHeight;

out vec4 finalColor;

void main()
{
    float x = gl_FragCoord.x / screenWidth;
    float y = gl_FragCoord.y / screenHeight;
    vec2 uv = vec2(x,y);
    
    vec3 Kd = texture(diffuse, uv).xyz;
    vec3 Ka = texture(ambient, uv).xyz;
    vec3 Ks = texture(specular, uv).xyz;
    vec3 Kdt = texture(diffuseTex, uv).xyz;
    vec3 Kat = texture(ambientTex, uv).xyz;
    vec3 N = texture(normal, uv).xyz;

    vec3 color = Ka*Kat*lightAmbient + Kd*Kdt*lightColor*max(0, dot(N, -lightDir));
    
    finalColor = vec4(color, 1.0);
}

