#version 450

layout(location = 0) in vec3 v_VertexColor;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 1) uniform sampler2D TestImage;

layout (set = 0, binding = 0) uniform UniformBuffer
{
    float ScreenWidth;
    float ScreenHeight;
    float AspectRatio;
    float Time;
} u_UniformBuffer;

void main() 
{
    vec2 texCoord = v_TexCoord;

    float time = u_UniformBuffer.Time;
    texCoord.x += 0.02 * sin(15 * v_TexCoord.x * v_TexCoord.y + time);

    o_Color = texture(TestImage, texCoord);

    o_Color.rgb *= v_VertexColor;
}