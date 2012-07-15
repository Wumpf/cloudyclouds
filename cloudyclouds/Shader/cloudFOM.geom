#version 330

// configuration
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// uniforms
uniform mat4 LightViewProjection;
uniform vec3 CameraRight;
uniform vec3 CameraUp;

// constants
const float alphaFadeOutFactor = 0.3;
const float alphaFadeInFactor = 0.35;
const float rotationSpeed = 0.05;
const float pi = 3.141592653589793;
const float depthDissortFactor = 0.5;

// input
in vec3 vs_out_position[1];
in float vs_out_size[1];
in float vs_out_remainingLifeTime[1];
in float vs_out_rand[1];

// output
out vec2 gs_out_texcoord;
out vec3 gs_out_worldPos;
out float gs_out_Alpha;
out float gs_out_depthDissort;

void main()
{
//	if(gl_PrimitiveIDIn != 0)
//	{ 

	// culling
	vec3 right = CameraRight * vs_out_size[0];
	vec3 up = CameraUp * vs_out_size[0];
	vec3 diag = right + up;

	vec3 uperRight_world = vs_out_position[0] + diag;
	vec4 uperRight_clip = LightViewProjection * vec4(uperRight_world, 1.0);

	vec3 lowerLeft_world = vs_out_position[0] - diag;
	vec4 lowerLeft_clip = LightViewProjection * vec4(lowerLeft_world, 1.0);

	vec4 screenCorMinMax = vec4(uperRight_clip.xy / uperRight_clip.w, lowerLeft_clip.xy / lowerLeft_clip.w);
	vec4 absScreenCorMinMax = abs(screenCorMinMax);
	if(all(greaterThan(absScreenCorMinMax.xz, vec2(1,1))) ||
	   all(greaterThan(absScreenCorMinMax.yw, vec2(1,1))))
	   return;

	// alpha
	gs_out_Alpha = min(min(vs_out_size[0] * alphaFadeInFactor, vs_out_remainingLifeTime[0] * alphaFadeOutFactor), 1.0);
	// size
	gs_out_depthDissort = vs_out_size[0] * depthDissortFactor;
	
	// texture animation
	float rotation = vs_out_remainingLifeTime[0] * rotationSpeed * sign(vs_out_rand[0]) + vs_out_rand[0] * pi;
	float cosRot = cos(rotation) * 0.5;
	float sinRot = sin(rotation) * 0.5;
	vec2 texRight	= vec2(cosRot, -sinRot);
	vec2 texUp		= vec2(sinRot, cosRot);
	vec2 texDiag	= texRight + texUp;

	// generate quad
	gl_Position.zw = vec2(uperRight_clip.z / uperRight_clip.w, 1.0);
	gl_Position.xy = screenCorMinMax.xy;
	gs_out_worldPos = uperRight_world;
	gs_out_texcoord = -texDiag + vec2(0.5,0.5);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.xw;
	gs_out_worldPos = vs_out_position[0] + right - up;
	gs_out_texcoord = texRight - texUp + vec2(0.5,0.5);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.zy;
	gs_out_worldPos = vs_out_position[0] - right + up;
	gs_out_texcoord = -texRight + texUp + vec2(0.5,0.5);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.zw;
	gs_out_worldPos = lowerLeft_world;
	gs_out_texcoord = texDiag + vec2(0.5,0.5);
	EmitVertex();
	EndPrimitive();
	/*}

	else
	{
	gs_out_Alpha = 1;
	gs_out_texcoord = vec2(0.5, 0.5);

	// generate quad
	gs_out_worldPos = vec3(0,20,30);
	gl_Position = LightViewProjection * vec4(gs_out_worldPos, 1);
	EmitVertex();

	gs_out_worldPos= vec3(30,20,0);
	gl_Position = LightViewProjection * vec4(gs_out_worldPos, 1);
	EmitVertex();

	gs_out_worldPos = vec3(0,20,0);
	gl_Position = LightViewProjection * vec4(gs_out_worldPos, 1);
	EmitVertex();
	EndPrimitive();
	} */
}
