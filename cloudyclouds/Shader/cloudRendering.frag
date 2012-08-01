#version 330

// uniforms
uniform sampler2D NoiseTexture;
uniform sampler2D FOMSampler0;
uniform sampler2D FOMSampler1;

uniform mat4 LightViewProjection;
uniform vec4 LightDistancePlane_norm;
uniform float LightFarPlane;

uniform vec3 LightDirection_viewspace;

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

// constants
const float twoPI = 6.28318531;
const vec3 ColorDark = vec3(0.34, 0.43, 0.51)*1.3;
const vec3 ColorLight = vec3(0.83, 0.83, 0.81)*1.1;
const vec3 ColorSpecular = vec3(0.95, 0.91, 0.84)*1.5;

// input
in vec3 gs_out_worldPos;
in vec2 gs_out_texcoord;
in float gs_out_Alpha;
in float gs_out_depthDissort;
in vec2 gs_out_relativePosition;

// output
out vec4 fragColor;


void main()
{	
	float tex = texture(NoiseTexture, gs_out_texcoord).a;
	float alpha = tex * gs_out_Alpha;
	if(alpha < 0.00001)
		discard;

	// sphere position
	vec3 worldPos = gs_out_worldPos - tex * CameraDir * gs_out_depthDissort;

	// fade at camera
	vec3 toViewer = gs_out_worldPos - CameraPosition;
	float toViewerLengthSq = dot(toViewer, toViewer);
	alpha *= min(toViewerLengthSq*0.001, 1.0); 

	// FOM
	vec4 posFOM = (LightViewProjection * vec4(worldPos, 1.0));
	vec2 texcoordFOM = (posFOM.xy / posFOM.w + vec2(1.0, 1.0)) * vec2(0.5, 0.5);
	vec4 coef0 = textureLod(FOMSampler0, texcoordFOM, 0.0); 
	vec4 coef1 = textureLod(FOMSampler1, texcoordFOM, 0.0);
/*
	coef0 += textureLod(FOMSampler0, texcoordFOM + vec2(1.0/512, 1.0/512), 0.0);
	coef1 += textureLod(FOMSampler1, texcoordFOM + vec2(1.0/512, 1.0/512), 0.0);

	coef0 += textureLod(FOMSampler0, texcoordFOM - vec2(1.0/512, 1.0/512), 0.0); 
	coef1 += textureLod(FOMSampler1, texcoordFOM - vec2(1.0/512, 1.0/512), 0.0);

	coef0 += textureLod(FOMSampler0, texcoordFOM - vec2(1.0/512, -1.0/512), 0.0); 
	coef1 += textureLod(FOMSampler1, texcoordFOM - vec2(1.0/512, -1.0/512), 0.0);

	coef0 += textureLod(FOMSampler0, texcoordFOM + vec2(1.0/512, -1.0/512), 0.0); 
	coef1 += textureLod(FOMSampler1, texcoordFOM + vec2(1.0/512, -1.0/512), 0.0);

	coef0 /= 5.0;
	coef1 /= 5.0;
	*/

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
	shadowing += (dot(avec, _a) + dot(bvec, _b)) / twoPI; 	// todo optimize

	// nice optimized (good visible)
	/*float cos1 = cos(twoPiDepth);
	float sin1 = sin(twoPiDepth);
	float cos2 = cos1*cos1 - sin1*sin1;
	float sin2 = sin1*cos1 + cos1*sin1;
	float cos3 = cos2*cos1 - sin2*sin1;
	float sin3 = sin2*cos1 + cos2*sin1;
	
	shadowing += (a1 * sin1       + a2 * sin2 / 2		+ a3 * sin3 / 3 +
				  b1 * (1.0-cos1) + b2 * (1.0-cos2) / 2  + b3 * (1.0-cos3) / 3) / twoPI;
	*/

	// direct brute
	/*shadowing += (a1 * sin1 +	    a2 * sin(twoPiDepth * 2) / 2  +	     	 a3 * sin(twoPiDepth * 3) / 3 +
				  b1 * (1.0-cos1) + b2 * (1.0-cos(twoPiDepth * 2)) / 2  +  b3 * (1.0-cos(twoPiDepth * 3)) / 3) 
					/ twoPI;*/

	shadowing = exp(-shadowing);


	// viewdir
	vec3 viewDir_viewspace = -normalize(ViewMatrix * vec4(worldPos, 1)).xyz;

	// blinn-phong specular - calc in viewspace
	vec3 normal_viewspace = vec3(-gs_out_relativePosition.yx,
								sqrt(1-dot(gs_out_relativePosition.xy, gs_out_relativePosition.xy)));	// normal on a sphere (dissorting by alpha seems not very effective)
	
	vec3 halfVector_viewspace = normalize(LightDirection_viewspace + viewDir_viewspace);
  	float specularAmount = dot(normal_viewspace, halfVector_viewspace); //max(dot(refl, viewDir_viewspace), 0);
	specularAmount *= specularAmount * specularAmount;	// spec exp is 2
	specularAmount *= alpha + 0.5;	// less spherical feeling by specularmapping per alpha

	// schlick fresnel - formula by http://filmicgames.com/archives/557
	float base = 1.0 - dot(LightDirection_viewspace, halfVector_viewspace);
	float exponential = base*base*base;//pow(base, 5.0);
	float fresnel = exponential + 0.1 * (1.0 - exponential);
	specularAmount *= fresnel;
	

	specularAmount *= shadowing;

	// final color
	fragColor.a = alpha;
	float NDotL = dot(LightDirection_viewspace, normal_viewspace);
	float lightAmount = min(1.0, shadowing + (shadowing + 0.6) * NDotL * alpha);	// here also retouch spherical terms with alpha
	fragColor.rgb = mix(ColorDark, ColorLight, lightAmount) + specularAmount*ColorSpecular;	// lerping between colors is important - clouds don't becom simply dark, they become dark blue!

	// visualize depth
//	fragColor = vec4(vec3(depth * depth * 8), alpha);
}