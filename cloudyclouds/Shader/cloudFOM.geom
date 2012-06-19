#version 330

// configuration
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// uniforms
uniform mat4 LightViewProjection;
uniform vec3 CameraRight;
uniform vec3 CameraUp;

// constants
const float alphaBlendLength = 0.8;

// input
in vec3 vs_out_position[1];
in float vs_out_size[1];
in float vs_out_remainingLifeTime[1];

// output
out vec2 gs_out_internPos;
out float gs_out_Alpha;
out float gs_out_Depth;

void main()
{
	// culling
	vec3 diag = (CameraRight + CameraUp) * vs_out_size[0];
	vec4 uperRight =  LightViewProjection * vec4(vs_out_position[0] + diag, 1.0);
	vec2 lowerLeft = (LightViewProjection * vec4(vs_out_position[0] - diag, 1.0)).xy;	// \todo transposing matrices and swaping matrices could save some ops
	vec4 screenCorMinMax = vec4(uperRight.xy, lowerLeft.xy);
	vec4 absScreenCorMinMax = abs(screenCorMinMax);
	if(all(greaterThan(absScreenCorMinMax.xz, uperRight.ww)) ||
	   all(greaterThan(absScreenCorMinMax.yw, uperRight.ww)))

	// alpha
	gs_out_Alpha = min(vs_out_remainingLifeTime[0] / alphaBlendLength, 1.0);
	// depth
	gs_out_Depth = (uperRight.z + 1.0) * 0.5;

	// generate quad
	gl_Position.zw = vec2(uperRight.z / uperRight.w, 1.0);
	gl_Position.xy = screenCorMinMax.xy;
	gs_out_internPos = vec2(-1.0, -1.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.xw;
	gs_out_internPos = vec2(-1.0, 1.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.zy;
	gs_out_internPos = vec2(1.0, -1.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.zw;
	gs_out_internPos = vec2(1.0, 1.0);
	EmitVertex();
	EndPrimitive();
}
