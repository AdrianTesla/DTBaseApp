#version 450

layout(location = 0) in vec3 v_LocalPos;
layout(location = 1) in vec3 v_WorldPos;
layout(location = 2) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 1) uniform sampler2D TestImage;

layout (set = 0, binding = 0) uniform UniformBuffer
{
    mat4 Projection;
    mat4 View;
    float ScreenWidth;
    float ScreenHeight;
    float AspectRatio;
    float Time;
} u_UniformBuffer;

void main() 
{
    float t = u_UniformBuffer.Time;

    vec3 ddx = dFdx(v_WorldPos);
    vec3 ddy = dFdy(v_WorldPos);
    vec3 normal = normalize(cross(ddx, ddy));

    float factor = dot(normal, vec3(0.0, 0.0, -1.0));

    o_Color = texture(TestImage, v_TexCoord);
    o_Color.rgb *= factor;

    o_Color.r /= pow(1.0 - length(v_LocalPos) * 2 / sqrt(3), 0.2 * (0.5 + 0.5 * sin(7 * t)));

    o_Color.rgb = vec3(248.0 / 255.0, 243.0 / 255.0, 122.0 / 255.0);
}