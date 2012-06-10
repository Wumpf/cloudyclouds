// geometry shader for growing hair

#version 330

// configuration
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// uniforms
layout(std140) uniform GlobalMatrices
{
	mat4 Projection;
	mat4 View;
	mat4 ViewProjection;
};

// input
struct Particle
{
	vec3 position;
};
in Particle vs_out[1];

// output


void main(void)
{
	// gen view aligned quad
	vec4 clipCordinate = ViewProjection * vec4(0.0, 1.0, 0.0, 1.0);
	vec2 sizeClipSpace = 1000 * vec2(1.0/1920.0, 1.0 / 1080.0);
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
