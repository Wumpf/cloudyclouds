#version 330

// input
layout(location = 0) in vec3 vs_in_position;
layout(location = 1) in float vs_in_size;
layout(location = 2) in float vs_in_remainingLifeTime;

// output
out vec3 vs_out_position;
out float vs_out_size;
out float vs_out_remainingLifeTime;

void main()
{	
	vs_out_position = vs_in_position;
	vs_out_size = vs_in_size;
	vs_out_remainingLifeTime = vs_in_remainingLifeTime;
}