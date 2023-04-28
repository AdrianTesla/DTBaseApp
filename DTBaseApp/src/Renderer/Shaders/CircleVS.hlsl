struct VSIn
{
    float2 Position : VertexPosition;
    float2 LocalPos : VertexLocalPos;
    float4 Color : VertexColor;
    float Thickness : CircleThickness;  
    float Fade : CircleFade;
};

struct VSOut
{
    float2 LocalPos : VertexLocalPos;
    float4 Color : VertexColor;
    float Thickness : CircleThickness;
    float Fade : CircleFade;
    float4 Position : SV_Position;
};

cbuffer UBCamera : register(b0)
{
    float4x4 ProjectionMatrix;
};

VSOut main(VSIn input)
{
    VSOut output;
    output.LocalPos = input.LocalPos;
    output.Thickness = input.Thickness;
    output.Fade = input.Fade;
    output.Color = input.Color;
    output.Position = mul(ProjectionMatrix, float4(input.Position.x, input.Position.y, 0.0f, 1.0f));
    return output;
}