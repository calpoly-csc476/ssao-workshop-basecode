#version  330 core

layout(location = 0) in vec3 vertPos;

out vec2 texCoord;

void main()
{
	gl_Position = vec4(vertPos, 1.0);
	texCoord = (vertPos.xy+vec2(1.0, 1.0))/2.0;
}
