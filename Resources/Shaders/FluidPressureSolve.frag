#version 330 core

in highp vec2 vUv;
in highp vec2 vL;
in highp vec2 vR;
in highp vec2 vT;
in highp vec2 vB;
//uniform float alpha;//alpha = -(dx)^2, where dx = grid cell size
uniform sampler2D uPressure;
uniform sampler2D uDivergence;
out highp vec4 FragColor;

void main ()
{
    float L = texture(uPressure, vL).x;
    float R = texture(uPressure, vR).x;
    float T = texture(uPressure, vT).x;
    float B = texture(uPressure, vB).x;
    float divergence = texture(uDivergence, vUv).x;
    float alpha = -1.0f;
    float rBeta = 0.25f;
    float pressure = (L + R + B + T + alpha * divergence) * rBeta;
    FragColor = vec4(pressure, 0.0, 0.0, 1.0);
}
