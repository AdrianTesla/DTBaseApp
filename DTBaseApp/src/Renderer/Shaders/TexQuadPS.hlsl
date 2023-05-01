struct PSIn
{
    float2 TexCoord : TexCoord;
    float4 Color : Color;
    float Tiling : Tiling;
    float4 Position : SV_Position;
};

SamplerState splr : register(s0);
Texture2D texture0 : register(t0);

float4 main(PSIn input) : SV_Target
{
    return texture0.Sample(splr, input.TexCoord * input.Tiling) * input.Color;
};