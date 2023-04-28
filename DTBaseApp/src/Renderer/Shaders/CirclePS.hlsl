struct PSIn
{
	float2 LocalPos : VertexLocalPos;
	float4 Color : VertexColor;
	float Thickness : CircleThickness;
    float Fade : CircleFade;
};

float4 main(PSIn i) : SV_TARGET
{
    float distance = length(i.LocalPos);
    
    i.Fade += 0.01;
    float q = 1.0;
    q *= smoothstep(1.0, 1.0 - i.Fade, distance);
    q *= smoothstep(1.0f - i.Thickness - i.Fade, 1.0f - i.Thickness, distance);
    
    return float4(i.Color.rgb, i.Color.a * q);
}