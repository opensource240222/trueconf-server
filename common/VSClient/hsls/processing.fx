
Texture2D tex2D;
Texture2D tex2D_0;
Texture2D tex2D_1;
Texture2D tex2D_2;

float dwSaturation;
float2 texelSize05;
float2 texelSize;
float2 texelSizeU;
float2 texelSizeV;
float2 imageSize;

SamplerState pointSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState linearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
	float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

struct PS_OUT 
{
	float4 Color : COLOR;
};

// csc matrix

static float4x4 cf_yuv2rgb =
{
	1.164, 0.000, 1.596, 0.000,
	1.164,-0.391,-0.813, 0.000,
	1.164, 2.018, 0.000, 0.000,
	0.000, 0.000, 0.000, 1.000
};

// bicubic resampler defines

#define	A -0.75

static float4x4	tco =
{
	0, A, -2*A, A,
	1, 0, -A-3, A+2,
	0, -A, 2*A+3, -A-2,
	0, 0, A, -A
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	
    output.Pos = input.Pos;
	output.Tex = input.Tex;
	
    return output;  
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

// point resampling

float4 PS_point( PS_INPUT input ) : SV_Target
{
    return tex2D.Sample( pointSampler, input.Tex ); 
}

// bilinear resampling

float4 PS_linear( PS_INPUT input ) : SV_Target
{
    return tex2D.Sample( linearSampler, input.Tex ); 
}

// for bicubic resampling

float4 taps(float t)
{
	return mul(tco, float4(1, t, t*t, t*t*t));
}

float4 Sample(float4 t, float2 samplePos, float2 sampleD)
{
	return
		mul(t, 
			float4x4(
				tex2D.Sample(pointSampler, samplePos - sampleD),
				tex2D.Sample(pointSampler, samplePos),
				tex2D.Sample(pointSampler, samplePos + sampleD),
				tex2D.Sample(pointSampler, samplePos + sampleD + sampleD)
			)
		);
}

// bicubic resampling (width)

float4 PS_resampler_bicubic_x(PS_INPUT input) : SV_Target
{
    float2 PixelPos = input.Tex;
	float2 dd = frac(PixelPos);
	float2 ExactPixel = PixelPos - dd;
	float2 samplePos = ExactPixel*texelSize + texelSize05;
	
	return Sample(taps(dd.x), samplePos, texelSizeU);
}

// bicubic resampling (height)

float4 PS_resampler_bicubic_y(PS_INPUT input) : SV_Target
{
    float2 PixelPos = input.Tex;
	float2 dd = frac(PixelPos);
	float2 ExactPixel = PixelPos - dd;
	float2 samplePos = ExactPixel*texelSize + texelSize05;
	
	return Sample(taps(dd.y), samplePos, texelSizeV);
}

// csc yuv444 -> rgba

PS_OUT PS_yuv444_to_rgba( PS_INPUT input ) : SV_Target
{
	PS_OUT output = (PS_OUT)0;
	
	float4 yuv = float4(tex2D_0.Sample(linearSampler, input.Tex).x - 0.0625,
					   (tex2D_1.Sample(linearSampler, input.Tex).x - 0.5) * dwSaturation,
					   (tex2D_2.Sample(linearSampler, input.Tex).x - 0.5) * dwSaturation,
						1.0);
	
	float y = 1.164 * yuv.x;
	float4 p;
	p.x = y + 1.596 * yuv.z;
	p.y = y - 0.391 * yuv.y - 0.813 * yuv.z;
	p.z = y + 2.018 * yuv.y;
	
	output.Color.r = clamp(p.x, 0, 1);
	output.Color.g = clamp(p.y, 0, 1);
	output.Color.b = clamp(p.z, 0, 1);
	output.Color.a = 1.0;

	return output;
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique10 vp_processing_frame_i420
{
    pass point_sampling
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_point() ) );
    }
    
    pass linear_sampling
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_linear() ) );
    }
    
    pass yuv444_to_rgba
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_yuv444_to_rgba() ) );
    } 

    pass resampler_bicubic_x
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_resampler_bicubic_x() ) );
    } 
    
    pass resampler_bicubic_y
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_resampler_bicubic_y() ) );
    }    
}
