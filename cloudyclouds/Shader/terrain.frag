#version 330

in vec3 normal;
in vec2 texture_coordinate;
in float height;
in float vertexShadowing;
in vec3 LightDirectionTangentSpace;

uniform sampler2D GrassTexture;
uniform sampler2D RockTexture; 
uniform sampler2D SnowTexture;
uniform sampler2D SandTexture;

uniform sampler2D GrassBump;
uniform sampler2D RockBump;
uniform sampler2D SnowBump;
uniform sampler2D SandBump;

uniform float SandHeight;
uniform float SnowHeight;

uniform vec3 LightDirectionObjSpace;

// texturing constants..
float RockGrasSmoothnes = 7.0;

float SnowTransitionArea = 1.0 / 5.0;	// lower means more transition ;)
float SnowElevationIndependance = 0.25;

float SandTransitionArea = 1.0 / 3.0;	// attention! the dividend has to be added to SandHeight
float SandElevationIndependance = 0.2;

// lighting/material constants
float AmbientAmount = 0.35;

out vec4 fragColor;

void main()
{
	// clear gourad shading problems up
	vec3 normalized_normal = normalize(normal);

	// blend between stone and snow by terrainelevation
	float elevation = abs(dot(normalized_normal, vec3(0.0, 1.0, 0.0)));
	float Grassnes = pow(elevation, RockGrasSmoothnes);
		// blend grass & Rock
	vec3 color = mix(texture2D(RockTexture, texture_coordinate).rgb,
					 texture2D(GrassTexture, texture_coordinate).rgb, Grassnes);

	// blend to snow
	float RockGrasInfluence = Grassnes + SnowElevationIndependance;
	float SnowTransition = clamp((height - SnowHeight) * SnowTransitionArea * RockGrasInfluence, 0.0, 1.0);
	color = mix(color, texture2D(SnowTexture, texture_coordinate).rgb, SnowTransition);

	// blend to Sand
	RockGrasInfluence = Grassnes + SandElevationIndependance;
	float SandTransition = clamp((SandHeight - height) * SandTransitionArea * RockGrasInfluence, 0.0, 1.0);
	color = mix(color, texture2D(SandTexture, texture_coordinate).rgb, SandTransition);

	// Lighting
	vec3 Bump = mix(texture2D(RockBump, texture_coordinate).rgb,
					  texture2D(GrassBump, texture_coordinate).rgb, Grassnes);
	Bump = mix(Bump, texture2D(SnowBump, texture_coordinate).rgb, SnowTransition);
	Bump = mix(Bump, texture2D(SandBump, texture_coordinate).rgb, SandTransition);
	Bump = normalize((Bump.rbg*2.0) - 1.0);

	float NdotL = dot(Bump, LightDirectionTangentSpace);

	NdotL *= vertexShadowing;
	color *= max(NdotL, 0.0) + AmbientAmount;

	// output
	fragColor = vec4(1,1,1,1);//color;
}