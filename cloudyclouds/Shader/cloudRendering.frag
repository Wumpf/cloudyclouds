#version 330

// uniforms
uniform mat4 LightViewProjection;
uniform sampler2D NoiseTexture;
uniform sampler2D FOMSampler0;
uniform sampler2D FOMSampler1;

// constants
const float twoPI = 6.28318531;

// input
in vec3 gs_out_worldPos;
in vec2 gs_out_internPos;
in float gs_out_Alpha;
in float gs_out_depth;

// output
out vec4 fragColor;


void main()
{	
	float alpha = (1.0 - dot(gs_out_internPos,gs_out_internPos)) * gs_out_Alpha;
	//if(alpha <= 0.001)
	//	discard;

	alpha *= texture(NoiseTexture, (gs_out_internPos+vec2(1.0, 1.0))*0.5).r;

	vec4 posFOM = LightViewProjection * vec4(gs_out_worldPos, 1.0);
	vec2 texcoordFOM = ((posFOM.xy / posFOM.w) + vec2(1.0, 1.0)) / 2.0;
	vec4 coef0 = textureLod(FOMSampler0, texcoordFOM, 0); 
	vec4 coef1 = textureLod(FOMSampler1, texcoordFOM, 0);

	#define a0 coef0.x
	#define a1 coef0.y
	#define b1 coef0.z
	#define a2 coef0.w
	#define b2 coef1.x
	#define a3 coef1.y
	#define b3 coef1.z

	float depth = posFOM.z;
	float shadowing = a0 / 2 * depth + 
				(a1 * sin(twoPI * depth) + a2 * sin(twoPI * depth * 2) * 2  +  a3 * sin(twoPI * depth * 3) * 3 +
				 b1 * (1.0-cos(twoPI * depth)) + b2 * (1.0-cos(twoPI * depth * 2)) * 2  +  b3 * (1.0-cos(twoPI * depth * 3)) * 3) / twoPI; // \todo optimize
	shadowing = exp(-shadowing);

	shadowing = shadowing*0.8 + 0.2;

	fragColor = vec4(shadowing,shadowing,shadowing, alpha);
}
