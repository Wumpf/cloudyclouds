#version 330

// uniforms
uniform mat4 ShadowViewProjection;
uniform texture 

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

    fragColor = vec4(gs_out_worldPos, alpha * gs_out_Alpha);
}
