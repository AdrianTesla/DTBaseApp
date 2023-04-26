struct VSOut
{
    float4 Color : VertexColor;
    float4 Position : SV_Position;
};

VSOut main(float2 position : VertexPosition, float4 color : VertexColor)
{
    VSOut output;
    output.Position = float4(position.x, position.y, 0.0f, 1.0f);
    output.Color = color;
    return output;
}