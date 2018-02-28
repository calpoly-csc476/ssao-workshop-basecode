#version 330 core

uniform sampler2D Texture0;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

in vec3 fPos;
in vec3 fragNor;
in vec2 vTexCoord;
in vec4 fPosLS;
in vec3 vColor;

void main()
{
	float amb = 0.3;
	float shade = 0.0;

	vec3 BaseColor = vColor;
	vec3 texColor0 = texture(Texture0, vTexCoord).rgb;

	outColor = amb * texColor0 + (1.0 - shade) * texColor0 * BaseColor;
	outNormal = fragNor * 0.5 + 0.5;
}
