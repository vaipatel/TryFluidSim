#version 330 core

in highp vec2 vUv;
uniform highp sampler2D uSource;
uniform highp float aspectRatio;
uniform highp vec3 color;
uniform highp vec2 point;
uniform highp float radius;
out vec4 FragColor;

void main(void)
{
    vec2 p = vUv - point.xy;
    p.x *= aspectRatio;
    vec3 splat = exp(-dot(p, p) / radius) * color;
    vec3 base = texture2D(uSource, vUv).xyz;
    FragColor = vec4(base + splat, 1.0);
}
