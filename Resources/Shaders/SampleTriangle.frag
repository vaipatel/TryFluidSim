#version 330 core

uniform float angle;
in vec4 col;
out vec4 FragColor1; // Pointed to by GL_COLOR_ATTACHMENT0 + 0. Will write to texture at m_handles[0] in texture unit GL_TEXTURE0 + 0.
out vec4 FragColor2; // Pointed to by GL_COLOR_ATTACHMENT0 + 1. Will write to texture at m_handles[1] in texture unit GL_TEXTURE0 + 1.

void main()
{
   FragColor1 = col;
   float g = col.y * cos(angle/60.0);
   float b = col.z * sin(angle/60.0);
   FragColor2 = vec4(col.x, g, b, 1);
}
