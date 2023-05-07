struct VSIn
{
    float2 Position : Position;
};

struct VSOut
{
    float4 Position : SV_Position;
};

VSOut main(VSIn input)
{
    VSOut output;
    output.Position = float4(input.Position.x, input.Position.y, 0.0f, 1.0f);
    return output;
}