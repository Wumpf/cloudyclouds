#version 330

// uniforms
uniform sampler3D VolumeTexture;

// input
in vec2 gs_out_internPos;
in float gs_out_Alpha;
in float gs_out_Depth;

// output
layout(location = 0) out vec4 coef0;
layout(location = 1) out vec4 coef1;

// constants
const float twoPI = 6.28318531;

void main()
{	
	float inverseAlpha = dot(gs_out_internPos, gs_out_internPos);
	if(inverseAlpha > 1.0)
		discard;

	inverseAlpha = 1.0f - (1.0f - inverseAlpha) * textureLod(VolumeTexture, vec3((gs_out_internPos+vec2(1.0, 1.0))*0.5, 0), 0).r;

	float twoPi_depth = twoPI * gs_out_Depth;
	
	#define a0 coef0.x
	#define a1 coef0.y
	#define b1 coef0.z
	#define a2 coef0.w
	#define b2 coef1.x
	#define a3 coef1.y
	#define b3 coef1.z

	a0 = -2.0 * log(inverseAlpha);

	a1 = cos(twoPi_depth);
	b1 = sin(twoPi_depth);

	a2 = a1*b1*2;		//cos(twoPi_depth * 2);
	b2 = b1*b1 + a1*a1; //sin(twoPi_depth * 2);

	a3 = b2*a1 + a2*b1; //cos(twoPi_depth * 3);
	b3 = a2*a1 + b2*b1; //sin(twoPi_depth * 3);

    coef0.yzw *= a0;
	coef1.xyz *= a0;
	coef1.w = gs_out_Depth;
}
