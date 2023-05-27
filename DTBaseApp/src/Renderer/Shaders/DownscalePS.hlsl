struct PSIn
{
    float2 TexCoord : TexCoord;
};

cbuffer DownscaleUB : register(b0)
{
    float2 TexelSize;
    uint IsFirstStage;
};

Texture2D<float3> PreviousStage : register(t0);
SamplerState splr : register(s0);

float Brightness(float3 v)
{
    return max(max(v.r, v.g), v.b);
}

float ToLinear(float x)
{
    if (x <= 0.04045f)
        return x / 12.92f;
    else
        return pow((x + 0.055f) / 1.055f, 2.4f);
}

float ToGamma(float x)
{
    if (x <= 0.0031308f)
        return 12.92f * x;
    else
        return 1.055f * pow(x, 0.41666f) - 0.055f;
}

float3 LinearToGamma(float3 color)
{
    return float3(
        ToGamma(color.x),
        ToGamma(color.y),
        ToGamma(color.z)
    );
}

float3 GammaToLinear(float3 color)
{
    return float3(
        ToLinear(color.x),
        ToLinear(color.y),
        ToLinear(color.z)
    );
}

float3 downsample_filter_high(float2 uv)
{
    // Downsample with a 4x4 box filter + anti-flicker filter
    float4 d = TexelSize.xyxy * float4(-1.0f, -1.0f, 1.0f, 1.0f);

    if(IsFirstStage == 1u)
    {
        float3 s1 = PreviousStage.Sample(splr, uv + d.xy);
        float3 s2 = PreviousStage.Sample(splr, uv + d.zy);
        float3 s3 = PreviousStage.Sample(splr, uv + d.xw);
        float3 s4 = PreviousStage.Sample(splr, uv + d.zw);
    
        // Karis's luma weighted average (using brightness instead of luma)
        float s1w = 1.0f / (Brightness(s1) + 1.0f);
        float s2w = 1.0f / (Brightness(s2) + 1.0f);
        float s3w = 1.0f / (Brightness(s3) + 1.0f);
        float s4w = 1.0f / (Brightness(s4) + 1.0f);
        float one_div_wsum = 1.0 / (s1w + s2w + s3w + s4w);
        
        return (s1 * s1w + s2 * s2w + s3 * s3w + s4 * s4w) * one_div_wsum;  
    }
    else
    {
        float3 s1 = PreviousStage.Sample(splr, uv + d.xy);
        float3 s2 = PreviousStage.Sample(splr, uv + d.zy);
        float3 s3 = PreviousStage.Sample(splr, uv + d.xw);
        float3 s4 = PreviousStage.Sample(splr, uv + d.zw);
    
        return (s1 + s2 + s3 + s4) * 0.25f;
    }
}

float4 main(PSIn input) : SV_Target
{
    return float4(downsample_filter_high(input.TexCoord), 1.0f);
}