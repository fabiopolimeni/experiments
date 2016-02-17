#version 450 core

#define FRAG_COLOR		0

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

in block
{
	vec3 Color;
	vec3 Position;
} In;

layout(location = FRAG_COLOR, index = 0) out vec4 FragColor;

void main()
{
	// vertex position and texture coordinates will be useful
	// when lines will bew drawn as screen space polygons.
	FragColor = vec4(In.Color.xyz, 1.0);
}
