// geometry shader for growing hair

#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform GlobalMatrices
{
	mat4 Projection;
	mat4 View;
	mat4 ViewProjection;
};

void main(void)
{
	gl_Position = ViewProjection * vec4(0, 0, 0, 1);
	EmitVertex();
	gl_Position = ViewProjection * vec4(10, 0, 0, 1);
	EmitVertex();
	gl_Position = ViewProjection * vec4(0, 10, 0, 1);
	EmitVertex();
	EndPrimitive();
}
