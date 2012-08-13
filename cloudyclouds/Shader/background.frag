#version 330

// uniforms
layout(std140) uniform View
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	mat4 InverseViewProjection;
	vec3 CameraPosition;
	vec3 CameraRight;
	vec3 CameraUp;
	vec3 CameraDir;
};
uniform vec3 LightDirection;

// input
in vec2 vs_out_texcoord;

// output
out vec4 fragColor;

// ------------------------------------------------
// SKY
// ------------------------------------------------
const vec3 AdditonalSunColor = vec3(1.0, 0.98, 0.8)/3;
const vec3 LowerHorizonColour = vec3(0.815, 1.141, 1.54)/2;
const vec3 UpperHorizonColour = vec3(0.986, 1.689, 2.845)/2;
const vec3 UpperSkyColour = vec3(0.16, 0.27, 0.43)*0.8;
const vec3 GroundColour = vec3(0.31, 0.41, 0.5)*0.8;
const float LowerHorizonHeight = -0.4;
const float UpperHorizonHeight = -0.1;
const float SunAttenuation = 2;
vec3 computeSkyColor(in vec3 ray)
{
	vec3 color;

	// background
	float heightValue = ray.y;	// mirror..
	if(heightValue < LowerHorizonHeight)
		color = mix(GroundColour, LowerHorizonColour, (heightValue+1) / (LowerHorizonHeight+1));
	else if(heightValue < UpperHorizonHeight)
		color = mix(LowerHorizonColour, UpperHorizonColour, (heightValue-LowerHorizonHeight) / (UpperHorizonHeight - LowerHorizonHeight));
	else
		color = mix(UpperHorizonColour, UpperSkyColour, (heightValue-UpperHorizonHeight) / (1.0-UpperHorizonHeight));
	
	// Sun
	float angle = max(0, dot(ray, LightDirection));
	color += (pow(angle, SunAttenuation) + pow(angle, 10000)*10) * AdditonalSunColor;

	return color;
}

// ------------------------------------------------
// TERRAIN
// ------------------------------------------------
const float terrainScale = 35;
const float minTerrainHeight = -65;
const float maxTerrainHeight = minTerrainHeight+terrainScale;

uniform sampler2D Heightmap;
const float heightmapTiling = 0.004;
const float texelSize = 1.0/512.0;

float getTerrainHeight(in vec2 pos, in float cameraDistance)
{
	float lod = min(9, cameraDistance*0.01);
	return textureLod(Heightmap, pos*heightmapTiling, lod).x * terrainScale + minTerrainHeight;
}

vec3 getTerrainNormal(in vec3 pos)
{
	float lod = 0.0;
	vec2 texcoord = pos.xz * heightmapTiling;
    vec3 n = vec3(textureLod(Heightmap, vec2(texcoord.x-texelSize, texcoord.y), lod).x - 
					textureLod(Heightmap, vec2(texcoord.x+texelSize, texcoord.y), lod).x,
                   texelSize*2 + textureLod(Heightmap, pos.xz*0.8, 0.0).x * texelSize,	// hehe some fine detail
				  textureLod(Heightmap, vec2(texcoord.x, texcoord.y-texelSize), lod).x - 
					textureLod(Heightmap, vec2(texcoord.x, texcoord.y+texelSize), lod).x  );
    return normalize(n);
}

// ------------------------------------------------
// RAYMARCH CORE
// ------------------------------------------------
bool rayCast(in vec3 rayOrigin, in vec3 rayDirection, out vec3 intersectionPoint, out float dist)
{
	if(rayDirection.y == 0)
		return false;

	// area
	const float maxStep = 200;
	float upperBound = (maxTerrainHeight - rayOrigin.y) / rayDirection.y;
	float lowerBound = (minTerrainHeight - rayOrigin.y) / rayDirection.y;
	if(lowerBound < 0.0 && upperBound < 0.0)
		return false;
	float start = max(min(upperBound, lowerBound), 5);
	float end   = min(max(upperBound, lowerBound), maxStep);

	// go!
//	float stepLen = 20;
	float lh = 0.0;
	float ly = 0.0;
	float lt = start;
	for(float t=start; t<end; t*=1.011)
	{
		vec3 pos = rayOrigin + rayDirection * t;
        float h = getTerrainHeight(pos.xz, t);

        if(pos.y - h < 0)
		{
			float laststep = t - lt;
			dist = (t - laststep + laststep*(lh-ly)/(pos.y-ly-h+lh));
			intersectionPoint = rayOrigin + rayDirection * dist;
			return true;
        }
		lt = t;
	//	stepLen = 0.012 * t;	// addaptive error
		lh = h;
		ly = pos.y;
	}
    return false;
}


// ------------------------------------------------
// FOM Shadowing
// ------------------------------------------------
uniform sampler2D FOMSampler0;
uniform sampler2D FOMSampler1;

uniform mat4 LightViewProjection;
uniform vec4 LightDistancePlane_norm;

const float twoPI = 6.28318531;

float FOMShadowing(in vec3 worldPos)
{
	vec4 posFOM = (LightViewProjection * vec4(worldPos, 1.0));
	vec2 texcoordFOM = (posFOM.xy / posFOM.w + vec2(1.0, 1.0)) * vec2(0.5, 0.5);
	vec4 coef0 = textureLod(FOMSampler0, texcoordFOM, 0.0); 
	vec4 coef1 = textureLod(FOMSampler1, texcoordFOM, 0.0);

	#define a0 coef0.x
	#define _a coef0.yzw
	#define _b coef1.xyz

	float depth = dot(LightDistancePlane_norm, vec4(worldPos, 1.0));
	float shadowing = a0 / 2 * depth;
	float twoPiDepth = twoPI * depth;

	vec3 avec;
	vec3 bvec;
	bvec.x = cos(twoPiDepth);
	avec.x = sin(twoPiDepth);
	bvec.y = bvec.x*bvec.x - avec.x*avec.x;
	avec.y = avec.x*bvec.x + bvec.x*avec.x;
	bvec.z = bvec.y*bvec.x - avec.y*avec.x;
	avec.z = avec.y*bvec.x + bvec.y*avec.x;
	bvec = vec3(1.0) - bvec;
	avec.y /= 2;
	bvec.y /= 2;
	avec.z /= 3;
	bvec.z /= 3;
	shadowing += (dot(avec, _a) + dot(bvec, _b)) / twoPI;

	return exp(-shadowing);
}


// ------------------------------------------------
// Coloring
// ------------------------------------------------
uniform sampler2D rockTexture;
uniform sampler2D grassTexture;
uniform sampler2D sandTexture;
vec3 getTerrainColor(in vec3 normal, in vec3 pos)
{
	// COLORING
	// blend between stone and snow by terrainelevation
	const float RockGrasSmoothnes = 0.5;
	const float SnowTransitionArea = 1.0 / 5.0;	// lower means more transition ;)
	const float SnowElevationIndependance = 0.25;
	const float SandTransitionArea = 1.0 / 3.0;	// attention! the dividend has to be added to SandHeight
	const float SandElevationIndependance = 0.2;
	const float SnowHeight = maxTerrainHeight-15;
	const float SandHeight = minTerrainHeight+10;
	const float TextureRepeat = 0.1;


	vec2 texcoord = pos.xz * TextureRepeat;

	float elevation = abs(dot(normal, vec3(0.0, 1.0, 0.0)));
	float Grassnes = pow(elevation, RockGrasSmoothnes);
		// blend grass & Rock
	vec3 color = mix(texture(rockTexture, texcoord).rgb, texture(grassTexture, texcoord).rgb, Grassnes);	// jep "texture" is not optimal; will be called outside any of these nifty dynamic branches
	// blend to snow
	float RockGrasInfluence = Grassnes + SnowElevationIndependance;
	float SnowTransition = clamp((pos.y - SnowHeight) * SnowTransitionArea * RockGrasInfluence, 0.0, 1.0);
	color = mix(color, vec3(0.8, 0.8, 0.8), SnowTransition);
	// blend to Sand
	RockGrasInfluence = Grassnes + SandElevationIndependance;
	float SandTransition = clamp((SandHeight - pos.y) * SandTransitionArea * RockGrasInfluence, 0.0, 1.0);
	color = mix(color, texture(sandTexture, texcoord).rgb, SandTransition);

	return color;
}

// ------------------------------------------------
// MAIN
// ------------------------------------------------
void main()
{	
	// "picking" - compute raydirection
	vec2 deviceCor = 2.0 * vs_out_texcoord - 1.0;
	vec4 rayOrigin = InverseViewProjection * vec4(deviceCor, -1, 1);
	rayOrigin.xyz /= rayOrigin.w;
	vec4 rayTarget = InverseViewProjection * vec4(deviceCor, 0, 1);
	rayTarget.xyz /= rayTarget.w;
	vec3 rayDirection = normalize(rayTarget.xyz - rayOrigin.xyz);
	
	// Color
	fragColor.a = 0.0;
	
	vec3 terrainPosition;
	vec2 terrainDerivates;
	float dist;
	//int steps = 0;
	if(rayCast(CameraPosition, rayDirection, terrainPosition, dist))
	{

		// LIGHTING
		vec3 normal = getTerrainNormal(terrainPosition);
		vec3 null;
		float lighting = max(0, dot(normal, LightDirection));
		float distToShadowCaster;
		if(rayCast(terrainPosition+LightDirection, LightDirection, null, distToShadowCaster))
			lighting *= min(max(0.2, distToShadowCaster*0.01), 1.0);
		lighting += FOMShadowing(terrainPosition);	// this is ambient!


		fragColor.rgb = getTerrainColor(normal, terrainPosition) * lighting;


		// FOGGING
		// clever fog http://www.iquilezles.org/www/articles/fog/fog.htm
		float fogAmount = min(1, 0.5 * exp(-CameraPosition.y  * 0.01) * (1.0 - exp( -dist*rayDirection.y* 0.01)) / rayDirection.y);
		fragColor.rgb = mix(fragColor.rgb, computeSkyColor(rayDirection), fogAmount);

		// DEPTH
		vec4 clipPos = ViewProjection * vec4(terrainPosition, 1.0);
		gl_FragDepth = (clipPos.z / clipPos.w + 1.0) / 2.0;
	}
	else
	{
		fragColor.rgb = computeSkyColor(rayDirection);
		gl_FragDepth = 1.0;
	}
}
