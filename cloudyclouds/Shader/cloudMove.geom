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

// constants
const vec3 spawnareaMin = vec3(-100, -10, -100);
const vec3 spawnareaSpan = vec3(200, 20, 200);
const float lifeTimeMin = 1.0;
const float lifeTimeSpan = 10.0;
const float growthFactor = 10; 
const float windFactor = 1.5;
const float thermicFactor = 0.5; 


// input
in vec3 vs_out_position[1];
in float vs_out_size[1];
in float vs_out_remainingLifeTime[1];

// output
out vec3 gs_out_position;
out float gs_out_size;
out float gs_out_remainingLifeTime;

// random hash from
// http://prideout.net/blog/?tag=transform-feedback
const float InverseMaxInt = 1.0 / 4294967295.0;
float randhash(uint seed, float b)
{
	uint i=(seed^12345391u)*2654435769u;
	i^=(i<<6u)^(i>>26u);
	i*=2654435769u;
	i+=(i<<5u)^(i>>12u);
	return float(b * i) * InverseMaxInt;
}

void main()
{
	gs_out_remainingLifeTime = vs_out_remainingLifeTime[0] - frameTimeDelta;

	// spawn new particle
	if(gs_out_remainingLifeTime < 0.0f)
	{
		uint seed = uint(totalTime * 10000.0) + uint(gl_PrimitiveIDIn);
		gs_out_position = spawnareaMin + vec3(randhash(seed, spawnareaSpan.x), randhash(++seed, spawnareaSpan.y), randhash(++seed, spawnareaSpan.z));
		gs_out_remainingLifeTime = lifeTimeMin + randhash(++seed, lifeTimeSpan);
		gs_out_size = 0;
	}
	else
	{
		gs_out_position = vs_out_position[0];
		gs_out_position.x -= frameTimeDelta * windFactor;
		gs_out_position.y += frameTimeDelta * thermicFactor;


		gs_out_size = vs_out_size[0] + frameTimeDelta * growthFactor / min(20, vs_out_size[0] + 0.2);
	}

	EmitVertex();
	EndPrimitive();
}