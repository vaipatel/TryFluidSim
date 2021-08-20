#version 330 core

in highp vec4 VertexPosition;
in lowp vec4 VertexColor;
out lowp vec4 col;
uniform highp mat4 matrix;

void main()
{
   col = VertexColor;
   gl_Position = matrix * VertexPosition;
}
