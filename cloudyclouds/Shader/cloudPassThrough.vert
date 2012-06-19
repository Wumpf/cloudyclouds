#version 330

// input
layout(location = 0) in vec3 vs_in_position;
layout(location = 1) in float vs_in_size;
layout(location = 2) in float vs_in_remainingLifeTime;
layout(location = 3) in float vs_in_depth;

// output
out vec3 vs_out_position;
out float vs_out_size;
out float vs_out_remainingLifeTime;
out float vs_out_depthviewspace;

void main()
{	
	vs_out_position = vs_in_position;
	vs_out_size = vs_in_size;
	vs_out_remainingLifeTime = vs_in_remainingLifeTime;
	vs_out_depthviewspace = vs_in_depth;
}