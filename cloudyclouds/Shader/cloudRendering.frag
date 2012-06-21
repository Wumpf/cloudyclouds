#version 330

// uniforms
uniform mat4 LightViewProjection;
uniform mat4 LightView;
uniform float LightFarPlane;
uniform sampler2D NoiseTexture;
uniform sampler2D FOMSampler0;
uniform sampler2D FOMSampler1;

layout(std140) uniform View
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	vec3 CameraPosition;
	vec3 CameraRight;
	vec3 CameraUp;
	vec3 CameraDir;
};

// constants
const float twoPI = 6.28318531;
const vec3 sunLight = vec3(1.0, 0.95, 0.9);

// input
in vec3 gs_out_worldPos;
in vec2 gs_out_texcoord;
in float gs_out_Alpha;
in float gs_out_depth;

// output
out vec4 fragColor;


void main()
{	
	float alpha = texture(NoiseTexture, gs_out_texcoord).a * gs_out_Alpha;
	if(alpha < 0.00001)
		discard;

	// fade at camera
	vec3 toViewer = gs_out_worldPos - CameraPosition;
	float toViewerLengthSq = dot(toViewer, toViewer);
	alpha *= min(toViewerLengthSq*0.001, 1.0); 

	// sphere position
	vec2 toMid = vec2(0.5, 0.5) - gs_out_texcoord;
	vec3 worldPos = gs_out_worldPos - sqrt(1 - dot(toMid, toMid)) * CameraDir;

	// todo: alpha, viwer

	// FOM
	vec4 posFOM = (LightViewProjection * vec4(worldPos, 1.0));
	vec2 texcoordFOM = (posFOM.xy / posFOM.w + vec2(1.0, 1.0)) * vec2(0.5, 0.5);
	vec4 coef0 = textureLod(FOMSampler0, texcoordFOM, 0); 
	vec4 coef1 = textureLod(FOMSampler1, texcoordFOM, 0);
	#define a0 coef0.x
	#define a1 coef0.y
	#define b1 coef0.z
	#define a2 coef0.w
	#define b2 coef1.x
	#define a3 coef1.y
	#define b3 coef1.z
	float depth = -(LightView * vec4(worldPos, 1.0)).z / LightFarPlane;
	float twoPiDepth = twoPI * depth;
	float shadowing = a0 / 2 * depth;
	shadowing += (a1 * sin(twoPiDepth) +	    a2 * sin(twoPiDepth * 2) / 2  +	     	 a3 * sin(twoPiDepth * 3) / 3 +
				  b1 * (1.0-cos(twoPiDepth)) + b2 * (1.0-cos(twoPiDepth * 2)) / 2  +  b3 * (1.0-cos(twoPiDepth * 3)) / 3) 
					/ twoPI;
	shadowing = clamp(exp(-shadowing) + 0.3, 0.0, 1.0);


	// additional lighting features


	fragColor = vec4(sunLight * shadowing, alpha);
}