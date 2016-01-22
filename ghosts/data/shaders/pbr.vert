#version 450 core
#extension GL_ARB_shader_storage_buffer_object : require

#define POSITION	0
#define NORMAL		1
#define TEXCOORDS	2

#define TRANSFORM	0
#define LIGHT		1

precision highp float;
precision highp int;
layout(std140, column_major) uniform;
layout(std430, column_major) buffer;

layout(binding = TRANSFORM) uniform transform
{
	mat4 MVP;
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
} Transform;

layout(binding = POSITION) buffer position
{
	vec4 value[];
} Position;

layout(binding = NORMAL) buffer normal
{
	vec4 value[];
} Normal;

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
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
} Out;

void main()
{	
	Out.TexCoords = TexCoords.value[gl_VertexID];
	Out.TexCoords.y = 1.0 - Out.TexCoords.y;

	Out.Normal = normalize(Transform.NormalMatrix * vec4(Normal.value[gl_VertexID].xyz, 0.0)).xyz;
	
	vec4 VertPos = vec4(Position.value[gl_VertexID].xyz, 1.0f);
	Out.Position = vec3(Transform.ModelViewMatrix * VertPos);
	
	gl_Position = Transform.MVP * VertPos;
}
