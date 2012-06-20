#version 330

// configuration
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// uniforms
layout(std140) uniform Screen
{
	mat4 ProjectionMatrix;
	vec2 InverseScreenResolution;
};

layout(std140) uniform View
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	vec3 CameraPosition;
	vec3 CameraRight;
	vec3 CameraUp;
};

// constants
const float alphaBlendLength = 0.8;

// input
in vec3 vs_out_position[1];
in float vs_out_size[1];
in float vs_out_remainingLifeTime[1];
//in float vs_out_depthviewspace[1];	// [-1, 1] depth

// output
out vec3 gs_out_worldPos;
out vec2 gs_out_internPos; 
out float gs_out_Alpha;
//out float gs_out_depth;

void main()
{
	// gen view aligned quad
	// http://www.flipcode.com/archives/Billboarding-Excerpt_From_iReal-Time_Renderingi_2E.shtml

	vec4 clipCordinate = ViewProjection * vec4(vs_out_position[0], 1.0);	// cordinate of particle position in clipspace
	vec2 sizeClipSpace = vec2(vs_out_size[0]*100.0, vs_out_size[0]*100.0) * InverseScreenResolution;

	vec3 right = CameraRight * vs_out_size[0];
	vec3 up = CameraUp * vs_out_size[0];
	vec3 diag = right + up;

	vec3 uperRight_world = vs_out_position[0] + diag;
	vec4 uperRight_clip = ViewProjection * vec4(uperRight_world, 1.0);

	vec3 lowerLeft_world = vs_out_position[0] - diag;
	vec4 lowerLeft_clip = ViewProjection * vec4(lowerLeft_world, 1.0);
	vec4 screenCorMinMax = vec4(uperRight_clip.xy / uperRight_clip.w, lowerLeft_clip.xy / lowerLeft_clip.w);


	// alpha
	gs_out_Alpha = min(vs_out_remainingLifeTime[0] / alphaBlendLength, 1.0);
	//gs_out_depth = vs_out_depthviewspace[0] * 0.1;

	// generate quad
	gl_Position.zw = vec2(uperRight_clip.z / uperRight_clip.w, 1.0);
	gl_Position.xy = screenCorMinMax.xy;
	gs_out_worldPos = uperRight_world;
	gs_out_internPos = vec2(-1.0, -1.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.xw;
	gs_out_worldPos = vs_out_position[0] + right - up;
	gs_out_internPos = vec2(-1.0, 1.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.zy;
	gs_out_worldPos = vs_out_position[0] - right + up;
	gs_out_internPos = vec2(1.0, -1.0);
	EmitVertex();
	gl_Position.xy = screenCorMinMax.zw;
	gs_out_worldPos = lowerLeft_world;
	gs_out_internPos = vec2(1.0, 1.0);
	EmitVertex();
	EndPrimitive();
}
