#version 450

layout(location = 0) in vec3 v_FragColor;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 1) uniform sampler2D TestImage;

void main() 
{
    o_Color = texture(TestImage, v_TexCoord);
}