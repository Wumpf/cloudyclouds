#include "stdafx.h"
#include "Clouds.h"
#include "ShaderObject.h"
#include "Utils.h"
#include "Vector3.h"
#include "Matrix4.h"

const char* Clouds::transformFeedbackVaryings[] = { "gs_out_position", "gs_out_size", "gs_out_remainingLifeTime" };
const unsigned int Clouds::maxNumCloudParticles = 1000;//16384;
const unsigned int Clouds::fourierOpacityMapSize = 1024;

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
	screenResolutionX(screenResolutionX),
	screenResolutionY(screenResolutionY),
	moveShader(new ShaderObject("Shader\\cloudPassThrough.vert", "", "Shader\\cloudMove.geom", transformFeedbackVaryings, 3)),
	fomShader(new ShaderObject("Shader\\cloudPassThrough.vert", "Shader\\cloudFOM.frag", "Shader\\cloudFOM.geom")),
	renderingShader(new ShaderObject("Shader\\cloudPassThrough.vert", "Shader\\cloudRendering.frag", "Shader\\cloudRendering.geom"))
{
	shaderSetup();
	fboSetup();
	bufferSetup();
	samplerSetup();
}

Clouds::~Clouds()
{
	glDeleteFramebuffers(1, &fourierOpacityMap_FBO);
	glDeleteTextures(1, &fourierOpacityMap_DepthBuffer);
	glDeleteTextures(2, fourierOpacityMap_Textures);
	glDeleteSamplers(1, &linearSampler_noMipMaps);
	glDeleteVertexArrays(1, &vao_cloudParticleBuffer_Read);
	glDeleteVertexArrays(1, &vao_cloudParticleBuffer_Write);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Read);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Write);
}

void Clouds::shaderSetup()
{
	// set ubo bindings, get uniform locations
		// move
	GLuint blockIndex = glGetUniformBlockIndex(moveShader->getProgram(), "Timings"); 
	glUniformBlockBinding(moveShader->getProgram(), blockIndex, 2);	// Timings binding=2
	checkGLError("settings cloudMove");
		
		// fom
	fomShaderUniformIndex_cameraX = glGetUniformLocation(fomShader->getProgram(), "CameraRight");
	fomShaderUniformIndex_cameraY = glGetUniformLocation(fomShader->getProgram(), "CameraUp");
	fomShaderUniformIndex_lightViewProjection = glGetUniformLocation(fomShader->getProgram(), "LightViewProjection");
	checkGLError("settings cloudFOM");

		// rendering
	blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "Screen"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 0);	// Screen binding=0
	blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "View"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 1);	// View binding=1

	renderingShaderUniformIndex_lightViewProjection = glGetUniformLocation(renderingShader->getProgram(), "LightViewProjection");
	
	renderingShader->useProgram();
	glUniform1i(glGetUniformLocation(fomShader->getProgram(), "FOMSampler0"), 0);
	glUniform1i(glGetUniformLocation(fomShader->getProgram(), "FOMSampler1"), 0);

	checkGLError("settings cloudRendering");
}

void Clouds::fboSetup()
{
	// Data Stuff
		// textures
	glGenTextures(2, fourierOpacityMap_Textures);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fourierOpacityMapSize, fourierOpacityMapSize, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fourierOpacityMapSize, fourierOpacityMapSize, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

		// depth buffer
	/*glGenTextures(1, &fourierOpacityMap_DepthBuffer);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_DepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, fourierOpacityMapSize, fourierOpacityMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE, 0);
	*/
	
		// fbo
	glGenFramebuffers(1, &fourierOpacityMap_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fourierOpacityMap_Textures[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fourierOpacityMap_Textures[1], 0);
//	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fourierOpacityMap_DepthBuffer, 0);
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, DrawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Clouds::samplerSetup()
{
	// sampler
	glGenSamplers(1, &linearSampler_noMipMaps);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// bind to all used stages
	glBindSampler(0, linearSampler_noMipMaps);
	glBindSampler(1, linearSampler_noMipMaps);
	glBindSampler(2, linearSampler_noMipMaps);
}

void Clouds::bufferSetup()
{
	// data objects
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



	GLuint vbo_cloudParticleRendering;
	GLuint ibo_cloudParticleRendering;
	GLuint vao_cloudParticleRendering;
	GLuint vbo_cloudParticleDepth;
}

void Clouds::display(float timeSinceLastFrame)
{
	glDisable(GL_DEPTH_TEST); 

	// move clouds
	glEnable(GL_RASTERIZER_DISCARD); 
	moveShader->useProgram();
	glBindVertexArray(vao_cloudParticleBuffer_Read);									// specify source
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_cloudParticleBuffer_Write);	// specify target
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, maxNumCloudParticles);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

	// render FOM
	fomShader->useProgram();
		// setup
	Matrix4 project = Matrix4::projectionOrthogonal(300, 300, 0, 500);
	Matrix4 view = Matrix4::camera(Vector3(0, 150, 0), Vector3(0, 0, 0), Vector3(1,0,0));
	Matrix4 lightViewProjection = view * project;
	glUniform3fv(fomShaderUniformIndex_cameraX, 1, Vector3(view.m11, view.m21, view.m31));
	glUniform3fv(fomShaderUniformIndex_cameraY, 1, Vector3(view.m12, view.m22, view.m32));
	glUniformMatrix4fv(fomShaderUniformIndex_lightViewProjection, 1, false, lightViewProjection);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	// additive blending
	glBindVertexArray(vao_cloudParticleBuffer_Write);
	glViewport(0,0, fourierOpacityMapSize, fourierOpacityMapSize); 
	glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_POINTS, 0, maxNumCloudParticles);
		// reset stuff
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0, screenResolutionX, screenResolutionY); 

	
	// render clouds
	renderingShader->useProgram();
	glUniformMatrix4fv(renderingShaderUniformIndex_lightViewProjection, 1, false, lightViewProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[1]);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(vao_cloudParticleBuffer_Write);
	
	glDrawArrays(GL_POINTS, 0, maxNumCloudParticles);

	glBindVertexArray(0);
	glDisable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);


	// rotate read/write buffer
	swap(vao_cloudParticleBuffer_Read, vao_cloudParticleBuffer_Write);
	swap(vbo_cloudParticleBuffer_Read, vbo_cloudParticleBuffer_Write);
}