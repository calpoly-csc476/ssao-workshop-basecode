#version 330 core

in vec2 texCoord;
out vec4 color;

uniform int uMode;

uniform mat4 P;

uniform sampler2D sceneColorTex;
uniform sampler2D sceneNormalsTex;
uniform sampler2D sceneDepthTex;

float linearizeDepth(in float depth)
{
	float zNear = 0.1;
	float zFar  = 100.0;
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

vec3 reconstructViewspacePosition(vec2 texCoord)
{
	float depth = texture(sceneDepthTex, texCoord).r;
	vec3 ndc = vec3(texCoord, depth) * 2.0 - vec3(1.0);
	vec4 view = inverse(P) * vec4(ndc, 1.0);
	return view.xyz / view.w;
}


uniform sampler2D noiseTex;

const int kernelSize = 64;
uniform vec3 sampleVectors[kernelSize];

float calculateOcclusion(vec3 normal)
{
	vec3 fragPos = reconstructViewspacePosition(texCoord); // fragment position in view space

	// Random vector (to orient hemisphere)
	vec2 noiseScale = vec2(textureSize(sceneDepthTex, 0)) / textureSize(noiseTex, 0);
	vec3 randomVec = vec3(texture(noiseTex, texCoord * noiseScale).xy, 0.0);

	// Tangent to view transform
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	const float radius = 1.0; // radius of the hemisphere around fragment
	// determines maximum distance of occluders, and depends on the units of your scene

	// Run samples
	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; ++i)
	{
		// Sample position in view space
		vec3 sample = fragPos + (TBN * sampleVectors[i]) * radius;

		// Transform sample to NDC (get texture coordinates)
		vec4 offset = vec4(sample, 1.0);
		offset = P * offset; // view to clip-space
		offset.xy /= offset.w; // clip to ndc
		offset.xy = offset.xy * 0.5 + 0.5; // transform to range 0.0 - 1.0

		if (offset.x > 1.0 || offset.y > 1.0 || offset.x < 0.0 || offset.y < 0.0)
		{
			continue; // skip samples outside of framebuffer
		}

		vec3 samplePosition = reconstructViewspacePosition(offset.xy);

		if (samplePosition.z >= sample.z) // check for crevice - occlusion check
		{
			if (abs(fragPos.z - samplePosition.z) < radius) // check within radius - range check
			{
				occlusion += 1.0; // distance weight governed by sample distribution
			}
		}
	}

	occlusion = 1.0 - (occlusion / float(kernelSize));
	return occlusion;
}

void main()
{
	color = vec4(vec3(0.5), 1.0);

	if (uMode == 0)
	{
		vec3 normal = texture(sceneNormalsTex, texCoord).rgb;
		normal = normal * 2.0 - vec3(1.0);
		float occlusion = calculateOcclusion(normal);

		color.rgb = vec3(occlusion);

		vec2 noiseScale = vec2(textureSize(sceneDepthTex, 0)) / textureSize(noiseTex, 0);
		// color.rg = texture(noiseTex, texCoord * noiseScale).xy;
		// color.rg = texture(noiseTex, texCoord).xy;
	}
	if (uMode == 1) // Color pass-thru
	{
		color.rgb = texture(sceneColorTex, texCoord).rgb;
	}
	else if (uMode == 2) // Debug Normals
	{
		color.rgb = texture(sceneNormalsTex, texCoord).rgb;
	}
	else if (uMode == 3) // Debug Depth
	{
		float depth = texture(sceneDepthTex, texCoord).r;
		float linDepth = linearizeDepth(depth);

		color.r = depth;
		color.g = linDepth;
		color.b = sin(linDepth * 3.1415 * 2.0);
	}
}
