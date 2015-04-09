#version 330

uniform sampler2D depth;
uniform float screenWidth;
uniform float screenHeight;

out vec4 outputF;

float LinearizeDepth(vec2 uv)
{
    float n = 1.0; // camera z near
    float f = 20.0; // camera z far
    float z = texture(depth, uv).x;
    return (n * z) / ( f - z * (f - n) );
}

void main()
{
    float x = gl_FragCoord.x / screenWidth;
    float y = gl_FragCoord.y / screenHeight;
    vec2 uv = vec2(x,y);
    float lin = LinearizeDepth(uv);
    if (texture(depth, uv).x == 1.0)
    {
        lin = 1.0;
    }
    outputF = vec4(1.0-lin, 0.0, 0.0, 1.0);
}

