#version 330 core

in highp vec2 vUv;
in highp vec2 vL;
in highp vec2 vR;
in highp vec2 vT;
in highp vec2 vB;
uniform highp sampler2D uVelocity;
out highp vec4 FragColor;

void main ()
{
    float L = texture(uVelocity, vL).x;
    float R = texture(uVelocity, vR).x;
    float T = texture(uVelocity, vT).y;
    float B = texture(uVelocity, vB).y;
    vec2 C = texture(uVelocity, vUv).xy;
    if (vL.x < 0.0) { L = -C.x; }
    if (vR.x > 1.0) { R = -C.x; }
    if (vT.y > 1.0) { T = -C.y; }
    if (vB.y < 0.0) { B = -C.y; }
    float div = 0.5 * (R - L + T - B);
    FragColor = vec4(div, 0.0, 0.0, 1.0);
}
