#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 3) out;

uniform sampler2D uSource;

#define M_PI 3.1415926535897932384626433832795

vec2 rotate(vec2 v, float a)
{
    float s = sin(a);
    float c = cos(a);
    mat2 m = mat2(c, s, -s, c);
    return m * v;
}

void main()
{
    vec2 pos = gl_in[0].gl_Position.xy;        // lies in [-1, 1] x [-1, 1]
    vec2 uvPos = (pos + vec2(1.0, 1.0)) * 0.5; // lies in [ 0, 1] x [ 0, 1]
    vec2 rawVel = texture(uSource, uvPos).xy;
    float scale = 0.0004;
    vec2 vel =  rawVel * scale;

    vec4 start = gl_in[0].gl_Position;
    gl_Position = start;
    EmitVertex();

    vec4 head = gl_in[0].gl_Position + vec4(vel.x, vel.y, 0.0, 0.0);
    gl_Position = head;
    EmitVertex();

    vec4 right = gl_in[0].gl_Position + vec4(vel.x, vel.y, 0.0, 0.0);
    vec2 rightRotVel = rotate(-vel, M_PI/4.0);
    right = right + vec4(rightRotVel.x, rightRotVel.y, 0.0, 0.0) * 0.5;
    gl_Position = right;
    EmitVertex();

    EndPrimitive();
}

