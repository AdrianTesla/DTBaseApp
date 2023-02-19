#version 450

layout(location = 0) out vec3 fragColor;

vec3 hsv(float hue, float s, float v)
{
	while (hue > 1.0)
		hue -= 1.0;
	while (hue < 0.0)
		hue += 1.0;

	// gray
	if (s == 0.0) 
		return vec3(v, v, v);

	// hanky panky
	hue = 6.0 * (hue - int(hue));
	int i = int(hue);
	float f = hue - float(i);
	float p = v * (1.0 - s);
	float q = v * (1.0 - s * f);
	float t = v * (1.0 - s * (1.0 - f));

	switch (i)
	{
		case 0:  return vec3(v, t, p);
		case 1:  return vec3(q, v, p);
		case 2:  return vec3(p, v, t);
		case 3:  return vec3(p, q, v);
		case 4:  return vec3(t, p, v);
		default: return vec3(v, p, q);
	}
}

layout (push_constant) uniform PushConstant
{
	float u_AspectRatio;
	float u_Time;
};

void main() 
{
    float pi = 3.14159265358979;
    float angle = float(gl_VertexIndex) * 2 * pi / 3;
    
    float x = cos(angle + 0.02 * u_Time);
    float y = sin(angle + 0.02 * u_Time);

	x /= u_AspectRatio;
    gl_Position = vec4(x, y, 0.0, 1.0);

	float hue = float(gl_VertexIndex) / 3.0;
    fragColor = hsv(hue + 0.1 * u_Time, 1.0, 1.0);
}