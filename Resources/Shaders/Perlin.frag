#version 330 core

in vec2 TexCoords;
uniform sampler2D PerlinNoise;
uniform sampler2D RedPepperStrip;
uniform float Time;
out vec4 FragColor;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec4 perlinColor = texture(PerlinNoise, TexCoords); //vec4(TexCoords.x, TexCoords.y, 0, 1);
    float u = perlinColor.r + sin(Time/2);
    u = fract(u);
    float v = 0.5;
    vec4 sampledColor = texture(RedPepperStrip, vec2(u, v));
    if ( sampledColor.g > 0 )
    {
        sampledColor = vec4(0,0,0,1);
    }
    FragColor = sampledColor;
}
