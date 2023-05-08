struct VSIn
{
    float2 Position : Position;
};

struct VSOut
{
    float2 TexCoord : TexCoord;
    float4 Position : SV_Position;
};

VSOut main(VSIn input)
{
    VSOut output;
    output.Position = float4(input.Position.x, input.Position.y, 0.0f, 1.0f);
    output.TexCoord = float2((input.Position.x + 1.0f) / 2.0f, (-input.Position.y + 1.0f) / 2.0f);
    return output;
}