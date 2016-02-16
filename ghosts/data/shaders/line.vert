#version 450 core
#extension GL_ARB_shader_storage_buffer_object : require

#define POSITION	0
#define COLOR		1
#define TEXCOORDS	2

#define TRANSFORM	0

precision highp float;
precision highp int;

layout(std140, column_major) uniform;
layout(std430, column_major) buffer;

layout(binding = TRANSFORM) uniform transform
{
	mat4 MVP;
	mat4 MV;
} Transforms;

layout(binding = POSITION) buffer position
{
	vec4 value[];
} Positions;

layout(binding = COLOR) buffer color
{
	vec4 value[];
} Colors;

layout(binding = TEXCOORDS) buffer texcoords
{
	vec2 value[];
} TexCoords;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec3 Color;
	vec3 Position;
	vec2 TexCoords;
} Out;

void main()
{
	// vertex position in view space coordinates
	vec4 VertPos = vec4(Positions.value[gl_VertexID].xyz, 1.0f);	
	Out.Position = vec3(Transforms.MV * VertPos);
	
	Out.Color = Colors.value[gl_VertexID].xyz;

	Out.TexCoords = TexCoords.value[gl_VertexID];
	Out.TexCoords.y = 1.0 - Out.TexCoords.y;
	
	gl_Position = Transforms.MVP * VertPos;
}
