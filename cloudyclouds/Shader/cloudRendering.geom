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

// input
in vec3 vsout_position[1];
in vec3 vsout_size[1];

// output



void main(void)
{
	vec4 pos = vec4(0.0, 0.0, 0.0, 1.0);

	// gen view aligned quad
		// http://www.flipcode.com/archives/Billboarding-Excerpt_From_iReal-Time_Renderingi_2E.shtml
	/*vec3 camDir = normalize(pos.xyz - CameraPosition);
	vec3 camUp = vec3(0,1,0);
	vec3 camRight = normalize(cross(camUp, camDir));
	camUp = normalize(cross(camDir, camRight));
	gl_Position.w = 1.0;
	gl_Position.xyz = pos + camRight + camUp;
	gl_Position = ViewProjection * gl_Position;
	EmitVertex();
	gl_Position.xyz = pos - camRight + camUp;
	gl_Position = ViewProjection * gl_Position;
	EmitVertex();
	gl_Position.xyz = pos + camRight - camUp;
	gl_Position = ViewProjection * gl_Position;
	EmitVertex();
	gl_Position.xyz = pos - camRight - camUp;
	gl_Position = ViewProjection * gl_Position;
	EmitVertex();*/
		// use fast clipspace variant
	vec4 clipCordinate = ViewProjection * vec4(0.0, 1.0, 0.0, 1.0);
	vec2 sizeClipSpace = 1000 * inverseScreenResolution;
	gl_Position = clipCordinate;
	gl_Position.xy -= sizeClipSpace;
	EmitVertex();
	gl_Position = clipCordinate;
	gl_Position.x -= sizeClipSpace.x; gl_Position.y += sizeClipSpace.y;
	EmitVertex();
	gl_Position = clipCordinate;
	gl_Position.x += sizeClipSpace.x; gl_Position.y -= sizeClipSpace.y;
	EmitVertex();
	gl_Position = clipCordinate;
	gl_Position.xy += sizeClipSpace;
	EmitVertex();

	EndPrimitive();
}
