#version 330 core

in vec4 VertexPosition;
in vec2 VertexTexCoords;
uniform highp mat4 matrix;
out vec2 TexCoords;

void main(void)
{
    gl_Position = matrix * VertexPosition;
    TexCoords = VertexTexCoords;
}
