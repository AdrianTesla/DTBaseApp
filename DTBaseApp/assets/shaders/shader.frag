#version 450

layout(location = 0) in vec3 v_Color;
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
    float time = u_UniformBuffer.Time * 0.2;
    vec2 texCoord = vec2(v_TexCoord.x * u_UniformBuffer.AspectRatio, v_TexCoord.y);

    const float pi = 3.14159265358979;
    const float frequency = 0.1;
    float angle = 0.0 * 2.0 * pi * frequency * time;
    mat2 rotation = mat2(
        cos(angle), -sin(angle),
        sin(angle), cos(angle)
    );

    vec2 r = vec2(texCoord.x - 0.5, texCoord.y - 0.5);

    float xDeviation = sin(40 * r.y + 10.0 * time);
    float yDeviation = sin(40 * r.x + 10.0 * time);

    texCoord.x += 0.01 * xDeviation;
    texCoord.y += 0.01 * yDeviation;

    float cx = 1.0 - pow(abs(sin(10 * r.x) + sin(10 * r.y) - xDeviation), 0.3);
    float cy = 1.0 - pow(abs(sin(10 * r.x) + sin(10 * r.y) - yDeviation), 0.3);

    float caustic = (cx + cy) / 2;

    vec3 color;
    color.r = caustic;
    color.g = caustic;
    color.b = caustic;

    o_Color = texture(TestImage, texCoord);

    o_Color.rgb *= v_Color + color;
}