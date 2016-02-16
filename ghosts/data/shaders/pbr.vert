#version 450 core
#extension GL_ARB_shader_storage_buffer_object : require

#define POSITION	0
#define NORMAL		1
#define TEXCOORDS	2
#define TANGENT		3

#define TRANSFORM	0
#define LIGHT		1

precision highp float;
precision highp int;

layout(std140, column_major) uniform;
layout(std430, column_major) buffer;

layout(binding = TRANSFORM) uniform transform
{
	mat4 MVP;
	mat4 MV;
	mat4 N;
} Transforms;

layout(binding = LIGHT) uniform light
{
	vec4 direction;
} Light;

layout(binding = POSITION) buffer position
{
	vec4 value[];
} Positions;

layout(binding = NORMAL) buffer normal
{
	vec4 value[];
} Normals;

layout(binding = TANGENT) buffer tangent
{
	vec4 value[];
} Tangents;

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
	vec3 LightDir;
	vec3 ViewDir;
} Out;

void main()
{
	vec3 normal = normalize(Transforms.N * vec4(Normals.value[gl_VertexID].xyz, 0.0)).xyz;
	vec3 tangent = normalize(Transforms.N * vec4(Tangents.value[gl_VertexID].xyz, 0.0)).xyz;

	// reconstruct bitangent from tangent and normal B = (N x T) * H
	float handedness = Tangents.value[gl_VertexID].w;
	vec3 bitangent = normalize(cross(normal, tangent)) * handedness;

	// vertex position in view space coordinates
	vec4 VertPos = vec4(Positions.value[gl_VertexID].xyz, 1.0f);
	vec3 position = vec3(Transforms.MV * VertPos);

	// position in view space
	Out.Position = position;

	// world to tangent space matrix
	mat3 tbn = transpose(mat3(tangent, bitangent, normal));

	// outputs in tangent space
	Out.Normal = normal;
	Out.ViewDir = tbn * normalize(-position);
	
	// light direction already comes in view space
	//Out.LightDir = tbn * normalize(-Light.direction.xyz);
	Out.LightDir = -Light.direction.xyz;

	Out.TexCoords = TexCoords.value[gl_VertexID];
	Out.TexCoords.y = 1.0 - Out.TexCoords.y;
	
	gl_Position = Transforms.MVP * VertPos;
}
