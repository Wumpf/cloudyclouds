#version 330

// input
in vec3 vsin_position;
in float vsin_size;

// output
out vec3 vsout_position;
out float vsout_size;


void main()
{	
	vsout_size = 2.0f;//vsin_position;
	vsout_position = vsin_position;
}