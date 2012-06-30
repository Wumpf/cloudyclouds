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
//uniform samplerCube cubeTexture;

// constants
const vec3 AdditonalSunColor = vec3(1.0, 0.98, 0.8)/2;
const vec3 LowerHorizonColour = vec3(0.815, 1.141, 1.54)/2;
const vec3 UpperHorizonColour = vec3(0.986, 1.689, 2.845)/2;
const vec3 UpperSkyColour = vec3(0.16, 0.27, 0.43);
const vec3 GroundColour = vec3(0.31, 0.41, 0.5);
const float LowerHorizonHeight = -0.4;
const float UpperHorizonHeight = -0.1;
const float SunAttenuation = 2;

/*
vec3 SkyColor = vec3(0.23, 0.38, 0.60);
vec3 HorizonColor = vec3(0.77, 0.97, 1.0);
vec3 SunColor = vec3(1.0, 0.98, 0.8)*4;
float HorizonSharpness = 3;
float SunFallOff = 0.2;
float SunSharpness = 5.0;
*/

// input
in vec2 vs_out_texcoord;

// output
out vec4 fragColor;

void main()
{	
	// "picking"
	vec2 deviceCor = 2.0 * vs_out_texcoord - 1.0;
	vec4 rayOrigin = InverseViewProjection * vec4(deviceCor, -1, 1);
	rayOrigin.xyz /= rayOrigin.w;
	vec4 rayTarget = InverseViewProjection * vec4(deviceCor, 0, 1);
	rayTarget.xyz /= rayTarget.w;
	vec3 rayDirection = normalize(rayTarget.xyz - rayOrigin.xyz);
	
	float heightValue = rayDirection.y;	// mirror..
	if(heightValue < LowerHorizonHeight)
		fragColor.rgb = mix(GroundColour, LowerHorizonColour, (heightValue+1) / (LowerHorizonHeight+1));
	else if(heightValue < UpperHorizonHeight)
		fragColor.rgb = mix(LowerHorizonColour, UpperHorizonColour, (heightValue-LowerHorizonHeight) / (UpperHorizonHeight - LowerHorizonHeight));
	else
		fragColor.rgb = mix(UpperHorizonColour, UpperSkyColour, (heightValue-UpperHorizonHeight) / (1.0-UpperHorizonHeight));
	
	// Sun
	vec3 SunLightDir = normalize(vec3(-1.0, 1.0, 0));
	float angle = max(0, dot(rayDirection, SunLightDir));
	fragColor.rgb += (pow(angle, SunAttenuation) + pow(angle, 10000)*10) * AdditonalSunColor;

/*	// sky
	fragColor.rgb = mix(HorizonColor, SkyColor, min(1.0, abs(rayDirection.y+0.2)*HorizonSharpness));
	fragColor.a = 1.0f;
   
	// sun
	float sunAngle = dot(rayDirection, normalize(vec3(0.5, 1.0, 0)));
	float sun = min(1.0, SunFallOff * pow(max(sunAngle, 0), SunSharpness));
	fragColor.rgb = mix(fragColor.rgb, SunColor, sun);*/

	//fragColor = texture(cubeTexture, rayDirection);
}