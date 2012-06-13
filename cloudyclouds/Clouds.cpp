#include "stdafx.h"
#include "Clouds.h"
#include "ShaderObject.h"
#include "Utils.h"
#include "Vector3.h"

const unsigned int Clouds::maxNumCloudParticles = 65536;

Clouds::Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY) :
	renderingShader(new ShaderObject("Shader\\cloudRendering.vert", "Shader\\cloudRendering.frag", "Shader\\cloudRendering.geom"))
{
	// 
	GLuint blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "Screen"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 0);	// Screen binding=0
	// 
	blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "View"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 1);	// View binding=1

	// generate buffers for cloud particles
	const unsigned int bufferSize = maxNumCloudParticles * sizeof(float) * 3;

	glGenBuffers(1, &cloudParticleBuffer_Read);
	glBindBuffer(GL_ARRAY_BUFFER, cloudParticleBuffer_Read);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

	glGenBuffers(1, &cloudParticleBuffer_Write);
	glBindBuffer(GL_ARRAY_BUFFER, cloudParticleBuffer_Write);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Clouds::~Clouds()
{
	glDeleteBuffers(1, &cloudParticleBuffer_Read);
	glDeleteBuffers(1, &cloudParticleBuffer_Write);
}

void Clouds::display(float timeSinceLastFrame)
{
	// rotate read/write buffer
	swap(cloudParticleBuffer_Read, cloudParticleBuffer_Write);

	// move clouds

	// render clouds
	glBindBuffer(GL_ARRAY_BUFFER, cloudParticleBuffer_Write);
	renderingShader->useProgram();
	glDrawArrays(GL_POINTS, 0, 1);
}