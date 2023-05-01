struct VSIn
{
	float2 Position : Position;
    float2 TexCoord : TexCoord;
    float4 Color : Color;
    float Tiling : Tiling;
};

struct VSOut
{
    float2 TexCoord : TexCoord;
    float4 Color : Color;
    float Tiling : Tiling;
    float4 Position : SV_Position;
};

cbuffer UBCamera : register(b0)
{
    float4x4 ProjectionMatrix;
};

VSOut main(VSIn i)
{
    VSOut output;
    output.TexCoord = i.TexCoord;
    output.Color = i.Color;
    output.Tiling = i.Tiling;
    output.Position = mul(ProjectionMatrix, float4(i.Position.x, i.Position.y, 0.0f, 1.0f));
    return output;
};