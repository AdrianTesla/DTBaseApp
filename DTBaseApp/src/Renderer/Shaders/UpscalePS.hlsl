Texture2D PreviousStage : register(t0);
SamplerState splr : register(s0);

cbuffer UpscaleUB : register(b0)
{
    float u_SampleScale;
};

float3 upsample_filter_high(float2 uv, float2 texelSize)
{
  /* 9-tap bilinear upsampler (tent filter) */
    float4 d = texelSize.xyxy * float4(1.0f, 1.0f, -1.0f, 0.0f) * u_SampleScale;

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
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    uint width;
    uint height;
    PreviousStage.GetDimensions(width, height);
    float2 texelSize = 1.0f / float2((float) width, (float) height);
    color.rgb = upsample_filter_high(input.TexCoord, texelSize);
    return color;
};