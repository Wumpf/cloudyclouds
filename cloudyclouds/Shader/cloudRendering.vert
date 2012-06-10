#version 330

// input
struct Particle
{
	vec3 position;
};
in Particle vs_in;

// output
out Particle vs_out;

void main()
{	
	vs_out = vs_in;
}