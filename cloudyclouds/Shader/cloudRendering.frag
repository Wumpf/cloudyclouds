#version 330

// uniforms
uniform mat4 LightViewProjection;
uniform sampler2D FOMSampler0;
uniform sampler2D FOMSampler1;

// input
in vec3 gs_out_worldPos;
in vec2 gs_out_internPos;
in float gs_out_Alpha;

// output
out vec4 fragColor;

void main()
{	
	float alpha = 1.0 - dot(gs_out_internPos,gs_out_internPos);
	if(alpha <= 0.001)
		discard;

	vec4 posFOM = LightViewProjection * vec4(gs_out_worldPos, 1.0);
	vec2 texcoordFOM = ((posFOM.xy / posFOM.w) + vec2(1.0, 1.0)) / 2.0;
	vec4 coef0 = textureLod(fomMap0, texcoordFOM, 0); 
	vec4 coef1 = textureLod(fomMap1, texcoordFOM, 0);

    fragColor = mod(coef0, 10)*0.1;//vec4(gs_out_worldPos, alpha * gs_out_Alpha);
}
