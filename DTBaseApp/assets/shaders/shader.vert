#version 450

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec3 a_Color;
layout (location = 2) in vec2 a_TexCoord;

layout (location = 0) out vec3 v_VertexColor;
layout (location = 1) out vec2 v_TexCoord;

layout (set = 0, binding = 0) uniform UniformBuffer
{
    float ScreenWidth;
    float ScreenHeight;
    float AspectRatio;
    float Time;
} u_UniformBuffer;

void main() 
{
    const float pi = 3.14159265358979;
    const float frequency = 0.01;
    float angle = 2.0 * pi * frequency * u_UniformBuffer.Time;
    
    mat2 rotation = mat2(
        cos(angle), -sin(angle),
        sin(angle), cos(angle)
    );

    vec2 position = rotation * a_Position;

	position.x /= u_UniformBuffer.AspectRatio;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);

    v_VertexColor = a_Color;
    v_TexCoord = a_TexCoord;
}