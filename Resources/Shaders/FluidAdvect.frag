#version 330 core

in highp vec2 vUv;
uniform highp sampler2D uVelocity;
uniform highp sampler2D uSource;
uniform highp vec2 texelSize;
//uniform highp vec2 dyeTexelSize;
uniform highp float dt;
uniform highp float dissipation;
out highp vec4 FragColor;

vec4 bilerp (sampler2D sam, vec2 uv, vec2 tsize) {
    vec2 st = uv / tsize - 0.5;
    vec2 iuv = floor(st);
    vec2 fuv = fract(st);
    vec4 a = texture(sam, (iuv + vec2(0.5, 0.5)) * tsize);
    vec4 b = texture(sam, (iuv + vec2(1.5, 0.5)) * tsize);
    vec4 c = texture(sam, (iuv + vec2(0.5, 1.5)) * tsize);
    vec4 d = texture(sam, (iuv + vec2(1.5, 1.5)) * tsize);
    return mix(mix(a, b, fuv.x), mix(c, d, fuv.x), fuv.y);
}

void main ()
{
//#ifdef MANUAL_FILTERING
//    vec2 coord = vUv - dt * bilerp(uVelocity, vUv, texelSize).xy * texelSize;
//    vec4 result = bilerp(uSource, coord, dyeTexelSize);
//#else
    vec2 coord = vUv - dt * texture(uVelocity, vUv).xy * texelSize;
    vec4 result = texture(uSource, coord);
//#endif
    float decay = 1.0 + dissipation * dt;
    FragColor = result / decay;
}
