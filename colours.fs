#version 330 core

#ifdef GL_ES
precision mediump float;
#endif

out vec4 outColor;
in vec4 fragColor;

uniform float u_mouse_x;
uniform float u_mouse_y;
uniform float u_width;
uniform float u_height;

void main(){
    float max = sqrt(pow(u_width, 2) + pow(u_height, 2));
    float mul = distance(vec2(u_mouse_x, u_mouse_y), gl_FragCoord.xy)/max;
    vec2 st = (gl_FragCoord.xy/u_width) * mul;
    outColor = vec4(st, fragColor.z, fragColor.a);
}