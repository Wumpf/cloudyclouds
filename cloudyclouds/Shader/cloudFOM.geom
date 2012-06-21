#version 330

// configuration
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// uniforms
uniform mat4 LightViewProjection;
uniform mat4 LightViewMatrix;
uniform float FarPlane;
uniform vec3 CameraRight;
uniform vec3 CameraUp;

// constants
const float alphaBlendLength = 0.8;
const float maxAlpha = 0.6;

// input
in vec3 vs_out_position[1];
in float vs_out_size[1];
in float vs_out_remainingLifeTime[1];

// output
out vec2 gs_out_texcoord;
out float gs_out_Alpha;

void main()
{
	if(gl_PrimitiveIDIn != 0)
	{

	// culling
	vec3 diag = (CameraRight + CameraUp) * vs_out_size[0];
	vec4 uperRight =  LightViewProjection * vec4(vs_out_position[0] + diag, 1.0);
	vec2 lowerLeft = (LightViewProjection * vec4(vs_out_position[0] - diag, 1.0)).xy;	// \todo transposing matrices and swaping matrices could save some ops
	vec4 screenCorMinMax = vec4(uperRight.xy, lowerLeft.xy);
	vec4 absScreenCorMinMax = abs(screenCorMinMax);
	if(all(greaterThan(absScreenCorMinMax.xz, uperRight.ww)) ||
	   all(greaterThan(absScreenCorMinMax.yw, uperRight.ww)))
	   return;

	// alpha
	gs_out_Alpha = min(vs_out_remainingLifeTime[0] / alphaBlendLength, maxAlpha);

	// generate quad
	gl_Position.zw = vec2(uperRight.z / uperRight.w, 1.0);
	gl_Position.xy = screenCorMinMax.xy;
	gs_out_texcoord = vec2(0.0, 0.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.xw;
	gs_out_texcoord = vec2(0.0, 1.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.zy;
	gs_out_texcoord = vec2(1.0, 0.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.zw;
	gs_out_texcoord = vec2(1.0, 1.0);
	EmitVertex();
	EndPrimitive();

	}

	else
	{
	gs_out_Alpha = 0.6;
	gs_out_texcoord = vec2(0.5, 0.5);

	// generate quad
	vec3 pos = vec3(0,20,30);
	gl_Position = LightViewProjection * vec4(pos, 1);
	EmitVertex();

	pos= vec3(30,20,0);
	gl_Position = LightViewProjection * vec4(pos, 1);
	EmitVertex();

	pos = vec3(0,20,0);
	gl_Position = LightViewProjection * vec4(pos, 1);
	EmitVertex();
	EndPrimitive();
	
	}
}
