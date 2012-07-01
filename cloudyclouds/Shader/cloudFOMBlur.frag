#version 330

// uniforms
uniform sampler2D FOMSampler0;
uniform sampler2D FOMSampler1;
uniform vec4 Offset;	// xy positive offset, zw negative offset

// constants
const vec2 Weights = vec2(0.5, 0.25); //const vec3 Weights = vec3(0.4, 0.2, 0.1);

// input
in vec2 vs_out_texcoord;

// output
layout(location = 0) out vec4 coef0;
layout(location = 1) out vec4 coef1;

void main()
{
	coef0 = texture(FOMSampler0, vs_out_texcoord) * Weights[0];
	coef1 = texture(FOMSampler1, vs_out_texcoord) * Weights[0];

	vec4 tex = vs_out_texcoord.xyxy + Offset;
	coef0 += (texture(FOMSampler0, tex.xy) + texture(FOMSampler0, tex.zw)) * Weights[1];
	coef1 += (texture(FOMSampler1, tex.xy) + texture(FOMSampler1, tex.zw)) * Weights[1];
/*
	tex += Offset;
	coef0 += (texture(FOMSampler0, tex.xy) + texture(FOMSampler0, tex.zw)) * Weights[2];
	coef1 += (texture(FOMSampler1, tex.xy) + texture(FOMSampler1, tex.zw)) * Weights[2]; */
}