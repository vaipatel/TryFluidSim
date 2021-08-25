#version 330 core

in vec2 TexCoords;
uniform sampler2D PerlinOut;
out vec4 FragColor;

void main()
{
    FragColor = texture(PerlinOut, TexCoords); //vec4(TexCoords.x, TexCoords.y, 0, 1);
}
