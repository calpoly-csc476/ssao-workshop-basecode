#version 330 core

uniform sampler2D Texture0;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

uniform vec3 materialColor;
uniform vec3 lightDir;

in vec3 fWSNormal;
in vec3 fVSNormal;

void main()
{
	float ambient = 0.3;
	float diffuse = 0.7;


	float shading = max(dot(normalize(fWSNormal), normalize(lightDir)), 0.0);

	outColor = (ambient + shading * diffuse) * materialColor;
	outNormal = normalize(fVSNormal) * 0.5 + 0.5;
}
