#include "stdafx.h"
#include "Clouds.h"
#include "ShaderObject.h"


Clouds::Clouds() :
	renderingShader(new ShaderObject("Shader\\cloudRendering.vert", "Shader\\cloudRendering.frag", "Shader\\cloudRendering.geom"))
{
	GLuint blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "GlobalMatrices"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 0);	// Global Matrices binding=0
}

Clouds::~Clouds()
{
}

void Clouds::display(float timeSinceLastFrame)
{
	renderingShader->useProgram();
	glDrawArrays(GL_POINTS, 0, 1);
}