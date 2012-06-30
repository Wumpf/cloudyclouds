#version 330

// uniforms
layout(std140) uniform View
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	mat4 InverseViewProjection;
	vec3 CameraPosition;
	vec3 CameraRight;
	vec3 CameraUp;
	vec3 CameraDir;
};
layout(std140) uniform Timings
{
	float totalTime;
	float frameTimeDelta;
};

// constants
const vec3 spawnareaMin = vec3(-200, -20, -200);
const vec3 spawnareaSpan = vec3(400, 20, 400);
const float lifeTimeMin = 6.0;
const float lifeTimeSpan = 25.0;
const float growthFactor = 5.0; 
const float windFactor = 0.3;
const float thermicFactor = 0.1; 

// input
layout(location = 0) in vec3 vs_in_position;
layout(location = 1) in vec3 vs_in_size_time_rand;
//layout(location = 3) in float vs_in_depth;
#define vs_in_size					vs_in_size_time_rand.x
#define vs_in_remainingLifeTime		vs_in_size_time_rand.y
#define vs_in_rand					vs_in_size_time_rand.z

// output
out vec3 vs_out_position;
out vec3 vs_out_size_time_rand;
out float vs_out_depthviewspace;		
#define vs_out_size					vs_out_size_time_rand.x
#define vs_out_remainingLifeTime	vs_out_size_time_rand.y
#define vs_out_rand					vs_out_size_time_rand.z

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
	// get older
	vs_out_remainingLifeTime = vs_in_remainingLifeTime - frameTimeDelta;
	
	// spawn new particle
	if(vs_out_remainingLifeTime < 0.0f)
	{
		uint seed = uint(totalTime * 10000.0) + uint(gl_VertexID);
		vs_out_position = spawnareaMin + vec3(randhash(seed, spawnareaSpan.x), randhash(++seed, spawnareaSpan.y), randhash(++seed, spawnareaSpan.z));

		// slightly discretize the positions to encourage cloud heaps
		//vs_out_position = floor(vs_out_position*0.1)*10;

		vs_out_remainingLifeTime = lifeTimeMin + randhash(++seed, lifeTimeSpan);
		vs_out_size = 0;
		vs_out_rand = randhash(++seed, 2.0) - 1.0;
	}
	else
	{
		// move
		vs_out_position = vs_in_position;
		vs_out_position.x -= frameTimeDelta * windFactor * (vs_in_rand+1.2);
		vs_out_position.y += frameTimeDelta * thermicFactor * vs_in_rand;

		// grow
		vs_out_size = vs_in_size + frameTimeDelta * growthFactor / (vs_in_size + 1);	// large particles grow slow

		// rand
		vs_out_rand = vs_in_rand;
	}

	// depth output, culling
	vec3 diag = (CameraRight + CameraUp) * vs_out_size;
	vec4 uperRight =  ViewProjection * vec4(vs_out_position + diag, 1.0);
	vec2 lowerLeft = (ViewProjection * vec4(vs_out_position - diag, 1.0)).xy;	// \todo transposing matrices and swaping matrices could save some ops
	vec4 screenCorMinMax = vec4(uperRight.xy, lowerLeft.xy);
	vec4 absScreenCorMinMax = abs(screenCorMinMax);
	if(all(greaterThan(absScreenCorMinMax.xz, uperRight.ww)) ||
	   all(greaterThan(absScreenCorMinMax.yw, uperRight.ww)))
	{
		vs_out_depthviewspace = 999999;
	}
	else
	{
		vs_out_depthviewspace = (ViewProjection * vec4(vs_out_position, 1)).z;	// \todo transposing matrices and swaping matrices could save some ops
		if(vs_out_depthviewspace < 0.0)
			vs_out_depthviewspace = 999999;
	}
}