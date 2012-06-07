// geometry shader for growing hair

#version 330

layout(triangles) in;

layout(std140) uniform GlobalMatrices
{
	mat4 Projection;
	mat4 View;
	mat4 ViewProjection;
};


void main(void)
{
}
