#version 330 core

in highp vec3 VertexPosition;
in highp vec2 VertexTexCoords;
out highp vec2 vUv;
out highp vec2 vL;
out highp vec2 vR;
out highp vec2 vT;
out highp vec2 vB;
uniform highp vec2 texelSize;

void main ()
{
     vUv = VertexTexCoords; // VertexPosition.xy * 0.5 + 0.5;
     vL = vUv - vec2(texelSize.x, 0.0);
     vR = vUv + vec2(texelSize.x, 0.0);
     vT = vUv + vec2(0.0, texelSize.y);
     vB = vUv - vec2(0.0, texelSize.y);
     gl_Position = vec4(VertexPosition.xy, 0.0, 1.0);
}
