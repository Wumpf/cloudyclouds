#version 330

// uniforms
layout(std140) uniform View
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	vec3 CameraPosition;
	vec3 CameraRight;
	vec3 CameraUp;
};
layout(std140) uniform Timings
{
	float totalTime;
	float frameTimeDelta;
};

// constants
const vec3 spawnareaMin = vec3(-100, -20, -100);
const vec3 spawnareaSpan = vec3(200, 40, 200);
const float lifeTimeMin = 1.0;
const float lifeTimeSpan = 10.0;
const float growthFactor = 2.0; 
const float windFactor = 1.5;
const float thermicFactor = 0.5; 


// input
layout(location = 0) in vec3 vs_in_position;
layout(location = 1) in float vs_in_size;
layout(location = 2) in float vs_in_remainingLifeTime;
//layout(location = 3) in float vs_in_depth;

// output
out vec3 vs_out_position;
out float vs_out_size;
out float vs_out_remainingLifeTime;
out float vs_out_depthviewspace;		
// optimization possibilty: separate buffers - depth in its own!
// also some compression could be useful

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
		vs_out_remainingLifeTime = lifeTimeMin + randhash(++seed, lifeTimeSpan);
		vs_out_size = 0;
	}
	else
	{
		// move
		vs_out_position = vs_in_position;
		vs_out_position.x -= frameTimeDelta * windFactor;
		vs_out_position.y += frameTimeDelta * thermicFactor;

		// grow
		vs_out_size = vs_in_size + frameTimeDelta * growthFactor / min(20, vs_in_size + 0.2);
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