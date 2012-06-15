#version 330

// configuration
layout(points) in;
layout(points, max_vertices = 1) out;

// uniforms
layout(std140) uniform Timings
{
	float totalTime;
	float frameTimeDelta;
};

// input
in vec3 vs_out_position[1];
in float vs_out_size[1];
in float vs_out_remainingLifeTime[1];

// output
out vec3 gs_out_position;
out float gs_out_size;
out float gs_out_remainingLifeTime;


void main()
{
	gs_out_position = vec3(0.0, sin(totalTime)*2, 0.0);//vs_out_position[0];
	gs_out_size = 1000;
	gs_out_remainingLifeTime = 1.0f;
	EmitVertex();
	EndPrimitive();
}