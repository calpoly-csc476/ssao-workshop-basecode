#version 330 core

in vec2 texCoord;
out vec4 color;

uniform int uMode;

uniform mat4 P;

uniform sampler2D sceneColorTex;
uniform sampler2D sceneNormalsTex;
uniform sampler2D sceneDepthTex;

/* Gives us a linear mapping of depth values from 0.0 to 1.0 */
/* Useful for debug visualization of depths - otherwise it's hard */
/* to see anything not close to the near plane */
float linearizeDepth(in float depth)
{
	float zNear = 0.1;
	float zFar  = 100.0;
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

vec3 reconstructViewspacePosition(in vec2 texCoord)
{
	return vec3(0.0);
}

vec3 viewspaceToNDC(in vec3 viewSpacePosition)
{
	return vec3(0.0);
}


uniform sampler2D noiseTex;

const int kernelSize = 64;
uniform vec3 sampleVectors[kernelSize];

float calculateOcclusion(vec3 normal)
{
	float occlusion = 1.0;

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
		color.rgb = texture(sceneColorTex, texCoord).rgb * occlusion;
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
