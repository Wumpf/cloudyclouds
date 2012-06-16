// geometry shader for growing hair

#version 330

// configuration
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// uniforms
layout(std140) uniform Screen
{
	mat4 ProjectionMatrix;
	vec2 inverseScreenResolution;
};

layout(std140) uniform View
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	vec3 CameraPosition;
};

// constants
const float alphaBlendLength = 0.8;

// input
in vec3 vs_out_position[1];
in float vs_out_size[1];
in float vs_out_remainingLifeTime[1];

// output
out vec2 gs_out_internPos;
out float gs_out_Alpha;


void main()
{
	// gen view aligned quad
	// http://www.flipcode.com/archives/Billboarding-Excerpt_From_iReal-Time_Renderingi_2E.shtml
	// use fast clipspace variant


	// culling
	vec4 clipCordinate = ViewProjection * vec4(vs_out_position[0], 1.0);	// cordinate of particle position in clipspace
	vec2 sizeClipSpace = vs_out_size[0] * inverseScreenResolution;
	vec4 screenCorMinMax = clipCordinate.xyxy + vec4(sizeClipSpace.xy, -sizeClipSpace.xy);
	screenCorMinMax /= clipCordinate.w;
	vec4 absScreenCorMinMax = abs(screenCorMinMax);
	if(all(greaterThan(absScreenCorMinMax.xz, vec2(1.0, 1.0))) ||
	   all(greaterThan(absScreenCorMinMax.yw, vec2(1.0, 1.0))))
		return;

	// alpha
	gs_out_Alpha = min(vs_out_remainingLifeTime[0] / alphaBlendLength, 1.0);

	// generate quad
	gl_Position.zw = vec2(clipCordinate.z / clipCordinate.w, 1.0);
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
