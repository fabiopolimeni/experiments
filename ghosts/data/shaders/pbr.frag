#version 450 core

#define LIGHT		1
#define FRAG_COLOR	0

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

layout(location = FRAG_COLOR, index = 0) out vec4 Color;

vec3 ads(vec3 Normal, vec3 LightDir, vec3 Position)
{
	vec3 n = normalize( Normal );
	vec3 s = normalize( -LightDir );
	vec3 v = normalize( -Position );
	vec3 r = reflect( -s, n );
	vec3 c = vec3(0.5, 0.7, 0.1);
	
	return (c * max( dot(s, n), 0.0 ) + vec3(0.9) * pow( max( dot(r,v), 0.0 ), 64 ) );
}

void main()
{
	Color = vec4(ads(In.Normal, Light.value.xyz, In.Position), 1.0);
}
