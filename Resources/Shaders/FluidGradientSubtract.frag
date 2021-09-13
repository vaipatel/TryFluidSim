#version 330 core

in highp vec2 vUv;
in highp vec2 vL;
in highp vec2 vR;
in highp vec2 vT;
in highp vec2 vB;
uniform highp sampler2D uPressure;
uniform highp sampler2D uVelocity;
out highp vec4 FragColor;

void main ()
{
    float L = texture(uPressure, vL).x;
    float R = texture(uPressure, vR).x;
    float T = texture(uPressure, vT).x;
    float B = texture(uPressure, vB).x;
    vec2 velocity = texture(uVelocity, vUv).xy;
    velocity.xy -= vec2(R - L, T - B);
    FragColor = vec4(velocity, 0.0, 1.0);
}
