cbuffer PrefilterUB : register(b0)
{
    float4 CurveThreshold;
    float Knee;
    float ClampIntensity;
};

Texture2D InputImage : register(t0);
SamplerState splr : register(s0);

struct PSIn
{
    float2 TexCoord : TexCoord;
};

/* 3-tap median filter */
float3 median(float3 a, float3 b, float3 c)
{
    return a + b + c - min(min(a, b), c) - max(max(a, b), c);
}

float3 safe_color(float3 color)
{
    return min(color, 65000.0f);
}

float max_v3(float3 v)
{
    return max(max(v.x, v.y), v.z);
}

float4 prefilter(float2 texCoords)
{
   uint width;
   uint height;
   InputImage.GetDimensions(width, height);
   float2 texelSize = 1.0f / float2((float) width, (float) height);
    
  /* Anti flicker */
   float3 d = texelSize.xyx * float3(1.0f, 1.0f, 0.0f);
   float3 s0 = safe_color(InputImage.Sample(splr, texCoords.xy, 0.0f).rgb);
   float3 s1 = safe_color(InputImage.Sample(splr, texCoords.xy - d.xz, 0.0f).rgb);
   float3 s2 = safe_color(InputImage.Sample(splr, texCoords.xy + d.xz, 0.0f).rgb);
   float3 s3 = safe_color(InputImage.Sample(splr, texCoords.xy - d.zy, 0.0f).rgb);
   float3 s4 = safe_color(InputImage.Sample(splr, texCoords.xy + d.zy, 0.0f).rgb);
   float3 m = median(median(s0.rgb, s1, s2), s3, s4);
    
  /* Pixel brightness */
    float br = max_v3(m);

  /* Under-threshold part: quadratic curve */
    float rq = clamp(br - CurveThreshold.x, 0.0f, CurveThreshold.y);
    rq = CurveThreshold.z * rq * rq;

  /* Combine and apply the brightness response curve. */
    m *= max(rq, br - CurveThreshold.w) / max(1e-5, br);

  /* Clamp pixel intensity if clamping enabled */
    if (ClampIntensity > 0.0)
    {
        br = max(1e-5, max_v3(m));
        m *= 1.0 - max(0.0, br - ClampIntensity) / br;
    }

    return float4(m, 1.0);
}

float4 main(PSIn input) : SV_Target
{
    return prefilter(input.TexCoord);
}