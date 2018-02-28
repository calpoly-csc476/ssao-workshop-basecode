#version 330 core

in vec2 texCoord;
out vec4 color;

uniform int uMode;

uniform sampler2D sceneColorTex;
uniform sampler2D sceneNormalsTex;
uniform sampler2D sceneDepthTex;

float LinearizeDepth(in float depth)
{
	float zNear = 0.1;
	float zFar  = 100.0;
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{
	color = vec4(vec3(0.5), 1.0);

	if (uMode == 0) // Color pass-thru
	{
		color.rgb = texture(sceneColorTex, texCoord).rgb;
	}
	else if (uMode == 1) // Debug Normals
	{
		color.rgb = texture(sceneNormalsTex, texCoord).rgb;
	}
	else if (uMode == 2) // Debug Depth
	{
		float depth = texture(sceneDepthTex, texCoord).r;
		float linDepth = LinearizeDepth(depth);
		color.r = depth;
		color.g = linDepth;
		color.b = sin(linDepth * 3.1415 * 2.0);
	}
}
