#version 330 core

precision highp float;
in vec3 VertexPosition;
in vec2 VertexTexCoords;
out vec2 vUv;
out vec2 vL;
out vec2 vR;
out vec2 vT;
out vec2 vB;
uniform vec2 texelSize;

void main ()
{
     vUv = VertexTexCoords; // VertexPosition.xy * 0.5 + 0.5;
     vL = vUv - vec2(texelSize.x, 0.0);
     vR = vUv + vec2(texelSize.x, 0.0);
     vT = vUv + vec2(0.0, texelSize.y);
     vB = vUv - vec2(0.0, texelSize.y);
     gl_Position = vec4(VertexPosition.xy, 0.0, 1.0);
}
