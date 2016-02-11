#version 450 core

#define LIGHT		1
#define FRAG_COLOR	0

#define DIFFUSE			0
#define SPECULAR		1
#define NORMAL			2
#define ROUGHNESS		3
#define DISPLACEMENT	4
#define ENVIRONMENT		5

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

in block
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	vec3 LightDir;
	vec3 ViewDir;
} In;

layout(binding = DIFFUSE) uniform sampler2D DiffuseMap;
layout(binding = SPECULAR) uniform sampler2D SpecularMap;
layout(binding = NORMAL) uniform sampler2D NormalMap;
layout(binding = ROUGHNESS) uniform sampler2D RoughnessMap;
layout(binding = DISPLACEMENT) uniform sampler2D HeightMap;

layout(location = FRAG_COLOR, index = 0) out vec4 Color;

vec3 ads(vec3 Normal, vec3 LightDir, vec3 ViewDir)
{
	vec3 n = normalize( Normal );
	vec3 l = normalize( LightDir );
	vec3 v = normalize( ViewDir );
	vec3 r = reflect( -l, n );

	vec3 albedo = texture2D(DiffuseMap, In.TexCoords).xyz;
	vec3 diffuse = albedo * max( dot(l, n), 0.0 );
	vec3 shininess = vec3(0.9);
	vec3 specular = shininess * pow( max( dot(r, v), 0.0 ), 64 );

	return diffuse;// + specular;
}

void main()
{
	//vec4 normal = 2.0 * texture( NormalMap, In.TexCoords ) - 1.0;
	Color = vec4(ads(In.Normal.xyz, In.LightDir, -In.Position), 1.0);
}
