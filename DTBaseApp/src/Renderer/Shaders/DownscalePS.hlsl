struct PSIn
{
    float2 TexCoord : TexCoord;
};

cbuffer DownscaleUB : register(b0)
{
    float2 TexelSize;
    uint IsFirstStage;
};

Texture2D PreviousStage : register(t0);
SamplerState splr : register(s0);

float max_v3(float3 v)
{
    return max(max(v.r, v.g), v.b);
}

float3 downsample_filter_high(float2 uv)
{
  /* Downsample with a 4x4 box filter + anti-flicker filter */
    float4 d = TexelSize.xyxy * float4(-1.0f, -1.0f, 1.0f, 1.0f);

    float3 s1 = PreviousStage.Sample(splr, uv + d.xy).rgb;
    float3 s2 = PreviousStage.Sample(splr, uv + d.zy).rgb;
    float3 s3 = PreviousStage.Sample(splr, uv + d.xw).rgb;
    float3 s4 = PreviousStage.Sample(splr, uv + d.zw).rgb;
    
    if(IsFirstStage == 1u)
    {
      /* Karis's luma weighted average (using brightness instead of luma) */
        float s1w = 1.0f / (max_v3(s1) + 1.0f);
        float s2w = 1.0f / (max_v3(s2) + 1.0f);
        float s3w = 1.0f / (max_v3(s3) + 1.0f);
        float s4w = 1.0f / (max_v3(s4) + 1.0f);
        float one_div_wsum = 1.0 / (s1w + s2w + s3w + s4w);
        
        return (s1 * s1w + s2 * s2w + s3 * s3w + s4 * s4w) * one_div_wsum;  
    }
    
    return (s1 + s2 + s3 + s4) * 0.25f;
}

float4 main(PSIn input) : SV_Target
{
    return float4(downsample_filter_high(input.TexCoord), 1.0f);
}