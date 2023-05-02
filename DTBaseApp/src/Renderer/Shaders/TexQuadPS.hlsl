struct PSIn
{
    float2 TexCoord : TexCoord;
    float4 Color : Color;
    float Tiling : Tiling;
    int TexIndex : TexIndex;
};

SamplerState splr : register(s0);
Texture2D textures[32] : register(t0);

float4 main(PSIn input) : SV_Target
{
    switch (input.TexIndex)
    {
        case 0 : return textures[0].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 1 : return textures[1].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 2 : return textures[2].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 3 : return textures[3].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 4 : return textures[4].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 5 : return textures[5].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 6 : return textures[6].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 7 : return textures[7].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 8 : return textures[8].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 9 : return textures[9].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 10 : return textures[10].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 11 : return textures[11].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 12 : return textures[12].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 13 : return textures[13].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 14 : return textures[14].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 15 : return textures[15].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 16 : return textures[16].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 17 : return textures[17].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 18 : return textures[18].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 19 : return textures[19].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 20 : return textures[20].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 21 : return textures[21].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 22 : return textures[22].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 23 : return textures[23].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 24 : return textures[24].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 25 : return textures[25].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 26 : return textures[26].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 27 : return textures[27].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 28 : return textures[28].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 29 : return textures[29].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 30 : return textures[30].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
        case 31 : return textures[31].Sample(splr, input.TexCoord * input.Tiling) * input.Color;
    }
    return float4(1.0f, 0.0f, 1.0f, 1.0f);
};