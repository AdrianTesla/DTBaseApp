#version 450

layout(location = 0) out vec3 fragColor;

layout (push_constant) uniform ScreenData
{
    float AspectRatio;
    float Time;
} u_Data;

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

void main() 
{
    float pi = 3.14159265358979;
    float angle = float(gl_VertexIndex) * 2 * pi / 3;
    
    float x = cos(angle + 0.1 * u_Data.Time);
    float y = sin(angle + 0.1 * u_Data.Time);

    x /= u_Data.AspectRatio;
    gl_Position = vec4(x, y, 0.0, 1.0);

	float hue = float(gl_VertexIndex) / 3.0;
    fragColor = hsv(hue + u_Data.Time * 0.1, 1.0, 1.0);
}