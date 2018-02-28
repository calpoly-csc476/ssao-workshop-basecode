#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 lightDir;

out vec3 fPos;
out vec3 fragNor;
out vec2 vTexCoord;
out vec4 fPosLS;
out vec3 vColor;

void main()
{
	/* First model transforms */
	gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

	/* the position in world coordinates */
	fPos = (M*vec4(vertPos, 1.0)).xyz;

	/* the normal */
	fragNor = (M*vec4(vertNor, 0.0)).xyz;

	/* pass through the texture coordinates to be interpolated */
	vTexCoord = vertTex;

	/* a color that could be blended - or be shading */
	vColor = vec3(max(dot(fragNor, normalize(lightDir)), 0));
}
