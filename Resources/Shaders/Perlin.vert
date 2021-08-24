#version 330 core

in vec3 VertexPosition;
in vec2 VertexTexCoords;
out vec2 TexCoords;

void main()
{
    gl_Position = vec4(VertexPosition.x, VertexPosition.y, VertexPosition.z, 1.0);
    TexCoords = VertexTexCoords;
}
