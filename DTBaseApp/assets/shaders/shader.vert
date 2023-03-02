#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Color;
layout (location = 2) in vec2 a_TexCoord;

layout (location = 0) out vec3 v_Color;
layout (location = 1) out vec3 v_LocalPos;
layout (location = 2) out vec3 v_WorldPos;

layout (set = 0, binding = 0) uniform UniformBuffer
{
    mat4 Projection;
    mat4 View;
    float ScreenWidth;
    float ScreenHeight;
    float AspectRatio;
    float Time;
} u_UniformBuffer;

layout (push_constant) uniform WorldMatrix
{
    mat4 World;
};

void main() 
{
    float time = u_UniformBuffer.Time;

    gl_Position = u_UniformBuffer.Projection * u_UniformBuffer.View * World * vec4(a_Position, 1.0);
    v_Color = a_Color;

    v_WorldPos = (World * vec4(a_Position, 1.0)).xyz;
    v_LocalPos = a_Position;
}