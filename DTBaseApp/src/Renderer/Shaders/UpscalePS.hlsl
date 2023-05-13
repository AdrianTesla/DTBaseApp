Texture2D PreviousStage : register(t0);
SamplerState splr : register(s0);

cbuffer UpscaleUB : register(b0)
{
    float2 TexelSize;
    float SampleScale;
};

float3 upsample_filter_high(float2 uv)
{
  /* 9-tap bilinear upsampler (tent filter) */
    float4 d = TexelSize.xyxy * float4(1.0f, 1.0f, -1.0f, 0.0f) * SampleScale;

    float3 s;
    s = PreviousStage.Sample(splr, uv - d.xy).rgb;
    s += PreviousStage.Sample(splr, uv - d.wy).rgb * 2.0f;
    s += PreviousStage.Sample(splr, uv - d.zy).rgb;  
    s += PreviousStage.Sample(splr, uv + d.zw).rgb * 2.0f;
    s += PreviousStage.Sample(splr, uv).rgb * 4.0f;
    s += PreviousStage.Sample(splr, uv + d.xw).rgb * 2.0f;
    s += PreviousStage.Sample(splr, uv + d.zy).rgb;  
    s += PreviousStage.Sample(splr, uv + d.wy).rgb * 2.0f;
    s += PreviousStage.Sample(splr, uv + d.xy).rgb;

    return s * (1.0f / 16.0f);
}

struct PSIn
{
    float2 TexCoord : TexCoord;
};

float4 main(PSIn input) : SV_Target
{
    return float4(upsample_filter_high(input.TexCoord), 1.0f);
};