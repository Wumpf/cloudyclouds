#version 330

// uniforms
uniform sampler2D NoiseTexture;
uniform vec4 LightDistancePlane_norm;
uniform vec3 CameraDir;

// input
in vec2 gs_out_texcoord;
in vec3 gs_out_worldPos;
in float gs_out_Alpha;

// output
layout(location = 0) out vec4 coef0;
layout(location = 1) out vec4 coef1;

// constants
const float twoPI = 6.28318531;

void main()
{
	float tex = texture(NoiseTexture, gs_out_texcoord).a;
	float alpha = tex * gs_out_Alpha;
	if(alpha < 0.00001)
		discard;
	float inverseAlpha = 1.0 - alpha;

	// sphere position
	vec3 worldPos = gs_out_worldPos - tex * CameraDir;
	float depth = dot(LightDistancePlane_norm, vec4(worldPos, 1.0));

	// compute coefficients
	#define a0 coef0.x
	#define a1 coef0.y
	#define a2 coef0.z
	#define a3 coef0.w
	#define b1 coef1.x
	#define b2 coef1.y
	#define b3 coef1.z

	float twoPi_depth = twoPI * depth;
	a0 = -2.0 * log(inverseAlpha);

	a1 = cos(twoPi_depth);
	b1 = sin(twoPi_depth);

	a2 = a1*a1 - b1*b1;	//cos(twoPi_depth * 2);
	b2 = b1*a1 + a1*b1; //sin(twoPi_depth * 2);

	a3 = a2*a1 - b2*b1; //cos(twoPi_depth * 3);
	b3 = b2*a1 + a2*b1; //sin(twoPi_depth * 3);

    coef0.yzw *= a0;
	coef1.xyz *= a0;
	coef1.w = 1;	// checking for debug purposes
}
