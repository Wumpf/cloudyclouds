#include "stdafx.h"
#include "Clouds.h"
#include "ShaderObject.h"
#include "Utils.h"
#include "Vector3.h"

const char* Clouds::transformFeedbackVaryings[] = { "gs_out_position", "gs_out_size", "gs_out_remainingLifeTime" };
const unsigned int Clouds::maxNumCloudParticles = 16384;

// struct representation of a particle vertex
struct ParticleVertex
{
	float position[3];
	float size;
	float lifetime;
};


#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(a) ((char*)NULL + (a))
#endif

GLuint Query = 0;

Clouds::Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY) :
	moveShader(new ShaderObject("Shader\\cloudPassThrough.vert", "", "Shader\\cloudMove.geom", transformFeedbackVaryings, 3)),
	renderingShader(new ShaderObject("Shader\\cloudPassThrough.vert", "Shader\\cloudRendering.frag", "Shader\\cloudRendering.geom"))
{
	// set ubo bindings
		// move
	GLuint blockIndex = glGetUniformBlockIndex(moveShader->getProgram(), "Timings"); 
	glUniformBlockBinding(moveShader->getProgram(), blockIndex, 2);	// Timings binding=2

		// rendering
	blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "Screen"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 0);	// Screen binding=0
	blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "View"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 1);	// View binding=1


	// vbo
		// init with random lifetimes as seed
	std::unique_ptr<ParticleVertex[]> startParticles(new ParticleVertex[maxNumCloudParticles]);
	for(int i=0; i<maxNumCloudParticles; ++i)
		startParticles[i].lifetime = random();

	glGenBuffers(1, &vbo_cloudParticleBuffer_Read);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Read);
		glBufferData(GL_ARRAY_BUFFER, maxNumCloudParticles * sizeof(ParticleVertex), startParticles.get(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkGLError("cloudVBO1");

	glGenBuffers(1, &vbo_cloudParticleBuffer_Write);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write);
		glBufferData(GL_ARRAY_BUFFER, maxNumCloudParticles * sizeof(ParticleVertex), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkGLError("cloudVBO2");

	// generate vao for cloud particles (2 for ping/pong)
	GLuint* vbo[] = { &vbo_cloudParticleBuffer_Read, &vbo_cloudParticleBuffer_Write };
	GLuint* vao[] = { &vao_cloudParticleBuffer_Read, &vao_cloudParticleBuffer_Write };
	for(int i=0; i<2; ++i)
	{ 

		// vao
		glGenVertexArrays(1, vao[i]);
		glBindVertexArray(*vao[i]);
			glBindBuffer(GL_ARRAY_BUFFER, *vbo[i]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), BUFFER_OFFSET(0));		// position
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), BUFFER_OFFSET(3*sizeof(float))); // size
			glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), BUFFER_OFFSET(4*sizeof(float))); // lifetime

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
		glBindVertexArray(0);
		checkGLError("cloudVAO");
	}


	glGenQueries(1, &Query);
}

Clouds::~Clouds()
{
	glDeleteVertexArrays(1, &vao_cloudParticleBuffer_Read);
	glDeleteVertexArrays(1, &vao_cloudParticleBuffer_Write);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Read);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Write);
}

void Clouds::display(float timeSinceLastFrame)
{
	/*static float f = 0;
	f+=timeSinceLastFrame;
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write);
	float a = 1000;
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*3, sizeof(float), &a);*/

	// move clouds
	glEnable(GL_RASTERIZER_DISCARD); 
	moveShader->useProgram();
		
	glBindVertexArray(vao_cloudParticleBuffer_Read);									// specify source
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_cloudParticleBuffer_Write);	// specify target
//glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, Query); 
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, maxNumCloudParticles);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

		/*glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN); 
		unsigned int PrimitivesWritten;
		glGetQueryObjectuiv(Query, GL_QUERY_RESULT, &PrimitivesWritten);
		std::cout << PrimitivesWritten << std::endl;*/
	
	// render clouds
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(vao_cloudParticleBuffer_Write);
	renderingShader->useProgram();
	glDrawArrays(GL_POINTS, 0, maxNumCloudParticles);
	glBindVertexArray(0);
	glDisable(GL_BLEND);

	// rotate read/write buffer
	swap(vao_cloudParticleBuffer_Read, vao_cloudParticleBuffer_Write);
	swap(vbo_cloudParticleBuffer_Read, vbo_cloudParticleBuffer_Write);
}