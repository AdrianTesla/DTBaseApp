struct PSIn
{
    float2 TexCoord : TexCoord;
};

Texture2D OriginalImage : register(t0);
Texture2D BloomLastStage : register(t1);
SamplerState splr : register(s0);

cbuffer CombineUB : register(b0)
{
    float BloomIntensity;
    float UpsampleScale;
}

float3 aces_tonemap(float3 color)
{
    float3x3 m1 = float3x3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
	);
    float3x3 m2 = float3x3(
        1.60475, -0.10208, -0.00327,
        -0.53108, 1.10813, -0.07276,
        -0.07367, -0.00605, 1.07602
	);
    float3 v = mul(m1, color);
    float3 a = v * (v + 0.0245786) - 0.000090537;
    float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return pow(clamp(mul(m2, (a / b)), 0.0, 1.0), 1.0 / 2.2);
}

// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    { 0.59719, 0.35458, 0.04823 },
    { 0.07600, 0.90834, 0.01566 },
    { 0.02840, 0.13383, 0.83777 }
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367 },
    { -0.10208, 1.10813, -0.00605 },
    { -0.00327, -0.07276, 1.07602 }
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

float3 tone_mapping_aces_filmic(float3 color)
{
    // Matrice di correzione gamma per la conversione da sRGB a scene-linear ACES
    float3x3 srgb_to_linear = float3x3(
        0.59719f, 0.35458f, 0.04823f,
        0.07600f, 0.90834f, 0.01566f,
        0.02840f, 0.13383f, 0.83777f
    );

    // Matrice di correzione gamma per la conversione da scene-linear ACES a sRGB
    float3x3 linear_to_srgb = float3x3(
        1.60475f, -0.53108f, -0.07367f,
        -0.10208f, 1.10813f, -0.00605f,
        -0.00327f, -0.07276f, 1.07602f
    );

    // Conversione da sRGB a scene-linear ACES
    float3 linear_color = mul(srgb_to_linear, color);

    // Adattamento dell'esposizione (esempio: nessuna modifica)
    float3 adapted_color = linear_color;

    // Conversione da scene-linear ACES a sRGB
    float3 srgb_color = mul(linear_to_srgb, adapted_color);

    // Applicazione della correzione per mantenere il colore nell'intervallo [0, 1]
    srgb_color = max(0.0f, min(1.0f, srgb_color));
    return srgb_color;
}

float3 upsample_filter_high(float2 uv)
{
    uint width;
    uint height;
    BloomLastStage.GetDimensions(width, height);
    float2 texelSize = 1.0f / float2((float) width, (float) height);
    
  /* 9-tap bilinear upsampler (tent filter) */
    float4 d = texelSize.xyxy * float4(1.0f, 1.0f, -1.0f, 0.0f) * UpsampleScale;

    float3 s;
    s = BloomLastStage.Sample(splr, uv - d.xy).rgb;
    s += BloomLastStage.Sample(splr, uv - d.wy).rgb * 2.0f;
    s += BloomLastStage.Sample(splr, uv - d.zy).rgb;
    s += BloomLastStage.Sample(splr, uv + d.zw).rgb * 2.0f;
    s += BloomLastStage.Sample(splr, uv).rgb * 4.0f;
    s += BloomLastStage.Sample(splr, uv + d.xw).rgb * 2.0f;
    s += BloomLastStage.Sample(splr, uv + d.zy).rgb;
    s += BloomLastStage.Sample(splr, uv + d.wy).rgb * 2.0f;
    s += BloomLastStage.Sample(splr, uv + d.xy).rgb; 

    return s * (1.0f / 16.0f);
}

float4 main(PSIn input) : SV_Target
{   
    float4 bloomedColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    bloomedColor.rgb = upsample_filter_high(input.TexCoord);
    
    float4 originalColor = OriginalImage.Sample(splr, input.TexCoord);
    float4 finalColor = originalColor + bloomedColor * BloomIntensity;
    
    //finalColor.rgb = aces_tonemap(finalColor.rgb);
    //finalColor.rgb = tone_mapping_aces_filmic(finalColor.rgb);
    finalColor.rgb = ACESFitted(finalColor.rgb * 2.0f);
    return finalColor;
};