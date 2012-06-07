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
	gl_Position =  vec4(-1, 1, 0, 1);
	EmitVertex();
	gl_Position =  vec4(1, 1, 0, 1);
	EmitVertex();
	gl_Position =  vec4(0.5, -1, 0, 1);
	EmitVertex();
	EndPrimitive();
}
