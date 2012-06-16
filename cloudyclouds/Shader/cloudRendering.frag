#version 330

// input
in vec2 gs_out_internPos;
in float gs_out_Alpha;

// output
out vec4 fragColor;

void main()
{	
	float alpha = 1.0 - dot(gs_out_internPos,gs_out_internPos);
	if(alpha <= 0.001)
		discard;

    fragColor = vec4(0.9, 0.9, 0.9, alpha * gs_out_Alpha);
}
