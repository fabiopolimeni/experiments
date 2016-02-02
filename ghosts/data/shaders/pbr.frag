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

layout(binding = LIGHT) uniform light
{
	vec4 value;
} Light;

in block
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
} In;

layout(binding = DIFFUSE) uniform sampler2D DiffuseMap;
layout(binding = SPECULAR) uniform sampler2D SpecularMap;
layout(binding = NORMAL) uniform sampler2D NormalMap;
layout(binding = ROUGHNESS) uniform sampler2D RoughnessMap;
layout(binding = DISPLACEMENT) uniform sampler2D HeightMap;

layout(location = FRAG_COLOR, index = 0) out vec4 Color;

vec3 ads(vec3 Normal, vec3 LightDir, vec3 Position)
{
	vec3 n = normalize( Normal );
	vec3 s = normalize( -LightDir );
	vec3 v = normalize( -Position );
	vec3 r = reflect( -s, n );

	vec3 albedo = texture2D(NormalMap, In.TexCoords).xyz;
	vec3 diffuse = albedo * max( dot(s, n), 0.0 );
	vec3 shiness = vec3(0.9);
	vec3 specular = shiness * pow( max( dot(r,v), 0.0 ), 64 );

	//return albedo;
	return ( diffuse + specular );
}

void main()
{
	Color = vec4(ads(In.Normal, Light.value.xyz, In.Position), 1.0);
}
