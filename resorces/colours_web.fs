#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec4 outColor;
in vec4 fragColor;

uniform float u_mouse_x;
uniform float u_mouse_y;
uniform float u_width;
uniform float u_height;
uniform float u_scale;

void main(){
    float max = sqrt(u_width*u_width + u_height*u_height);
    float mul = (distance(vec2(u_mouse_x, u_mouse_y), gl_FragCoord.xy)/max)/max;
    vec2 st = (gl_FragCoord.xy/u_width) * abs(u_scale)*0.001f;
    outColor = vec4(st, fragColor.z, fragColor.a);
}