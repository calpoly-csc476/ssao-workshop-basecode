#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 fWSNormal;
out vec3 fVSNormal;

void main()
{
	/* First model transforms */
	gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

	/* the normal in worldspace */
	fWSNormal = (M * vec4(normalize(vertNor), 0.0)).xyz;

	/* the normal in viewspace */
	fVSNormal = (V * vec4(fWSNormal, 0.0)).xyz;
}
