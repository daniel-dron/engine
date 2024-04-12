#version 430 core
out vec4 FragColor;
  
in vec2 TexCoords;

layout(binding = 0) uniform sampler2D screenTexture;
layout(binding = 1) uniform sampler2D bloomBlur;

vec3 amd(vec3 color) {
  const float hdrMax = 16.0;
  const float contrast = 2.0;
  const float shoulder = 1.0;
  const float midIn = 0.18;
  const float midOut = 0.18;
  
  float b = (-pow(midIn, contrast) + pow(hdrMax, contrast) * midOut) / ((pow(pow(hdrMax, contrast), shoulder) - pow(pow(midIn, contrast), shoulder)) * midOut);
  float c = (pow(pow(hdrMax, contrast), shoulder) * pow(midIn, contrast) - pow(hdrMax, contrast) * pow(pow(midIn, contrast), shoulder) * midOut) / ((pow(pow(hdrMax, contrast), shoulder) - pow(pow(midIn, contrast), shoulder)) * midOut);

  const float EPS = 1e-6f;
  float peak = max(color.r, max(color.g, color.b));
  peak = max(EPS, peak);

  vec3 ratio = color / peak;
  vec4 p = vec4(contrast, shoulder, b, c);
  float z = pow(peak, p.r);
  peak = z / (pow(z, p.g) * p.b + p.a);

  float crosstalk = 4.0;
  float saturation = contrast;
  float crossSaturation = contrast * 16.0;

  float white = 1.0;

  ratio = pow(abs(ratio), vec3(saturation / crossSaturation));
  ratio = mix(ratio, vec3(white), vec3(pow(peak, crosstalk)));
  ratio = pow(abs(ratio), vec3(crossSaturation));

  color = peak * ratio;
  return color;
}

uniform float luma_threshold = 0.5f;
uniform float mul_reduce = 1.0f / 8.0f;
uniform float min_reduce = 1.0f / 128.0f;
uniform float max_span = 8.0f;
uniform vec2 texel_step = vec2(1.0f/1920, 1.0f/1080);

uniform bool use_fxaa = true;

vec3 fxaa() {
	vec3 rgbM = texture(screenTexture, TexCoords).rgb;
	vec3 out_color = vec3(1.0f);

	// Sampling neighbour texels. Offsets are adapted to OpenGL texture coordinates. 
	vec3 rgbNW = textureOffset(screenTexture, TexCoords, ivec2(-1, 1)).rgb;
    vec3 rgbNE = textureOffset(screenTexture, TexCoords, ivec2(1, 1)).rgb;
    vec3 rgbSW = textureOffset(screenTexture, TexCoords, ivec2(-1, -1)).rgb;
    vec3 rgbSE = textureOffset(screenTexture, TexCoords, ivec2(1, -1)).rgb;

	// see http://en.wikipedia.org/wiki/Grayscale
	const vec3 toLuma = vec3(0.299, 0.587, 0.114);
	
	// Convert from RGB to luma.
	float lumaNW = dot(rgbNW, toLuma);
	float lumaNE = dot(rgbNE, toLuma);
	float lumaSW = dot(rgbSW, toLuma);
	float lumaSE = dot(rgbSE, toLuma);
	float lumaM = dot(rgbM, toLuma);

	// Gather minimum and maximum luma.
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
	// If contrast is lower than a maximum threshold ...
	if (lumaMax - lumaMin <= lumaMax * luma_threshold)
	{
		// ... do no AA and return.
		out_color = rgbM;
		return out_color;
	}  
	
	// Sampling is done along the gradient.
	vec2 samplingDirection;	
	samplingDirection.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    samplingDirection.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    // Sampling step distance depends on the luma: The brighter the sampled texels, the smaller the final sampling step direction.
    // This results, that brighter areas are less blurred/more sharper than dark areas.  
    float samplingDirectionReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * mul_reduce, min_reduce);

	// Factor for norming the sampling direction plus adding the brightness influence. 
	float minSamplingDirectionFactor = 1.0 / (min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);
    
    // Calculate final sampling direction vector by reducing, clamping to a range and finally adapting to the texture size. 
    samplingDirection = clamp(samplingDirection * minSamplingDirectionFactor, vec2(-max_span), vec2(max_span)) * texel_step;
	
	// Inner samples on the tab.
	vec3 rgbSampleNeg = texture(screenTexture, TexCoords+ samplingDirection * (1.0/3.0 - 0.5)).rgb;
	vec3 rgbSamplePos = texture(screenTexture, TexCoords + samplingDirection * (2.0/3.0 - 0.5)).rgb;

	vec3 rgbTwoTab = (rgbSamplePos + rgbSampleNeg) * 0.5;  

	// Outer samples on the tab.
	vec3 rgbSampleNegOuter = texture(screenTexture, TexCoords + samplingDirection * (0.0/3.0 - 0.5)).rgb;
	vec3 rgbSamplePosOuter = texture(screenTexture, TexCoords + samplingDirection * (3.0/3.0 - 0.5)).rgb;
	
	vec3 rgbFourTab = (rgbSamplePosOuter + rgbSampleNegOuter) * 0.25 + rgbTwoTab * 0.5;   
	
	// Calculate luma for checking against the minimum and maximum value.
	float lumaFourTab = dot(rgbFourTab, toLuma);
	
	// Are outer samples of the tab beyond the edge ... 
	if (lumaFourTab < lumaMin || lumaFourTab > lumaMax)
	{
		// ... yes, so use only two samples.
		out_color = rgbTwoTab; 
	}
	else
	{
		// ... no, so use four samples. 
		out_color = rgbFourTab;
	}

	return out_color;
}

void main()
{ 
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
	vec3 out_color = hdrColor;

	if (use_fxaa) {
		out_color = fxaa();
	}

	out_color = pow(out_color, vec3(1.0 / 2.4f));
	out_color = amd(out_color);

	FragColor = vec4(out_color, 1.0f);
}
