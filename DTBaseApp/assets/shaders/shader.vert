#version 450

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec3 a_Color;

layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform PushConstant
{
	float u_AspectRatio;
	float u_Time;
};

void main() 
{
    const float pi = 3.14159265358979;
    const float frequency = 0.01;
    float angle = 2.0 * pi * frequency * u_Time;
    
    mat2 rotation = mat2(
        cos(angle), -sin(angle),
        sin(angle), cos(angle)
    );

    vec2 position = rotation * a_Position;

	position.x /= u_AspectRatio;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);

    fragColor = a_Color;
}