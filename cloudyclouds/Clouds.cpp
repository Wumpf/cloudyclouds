﻿#include "stdafx.h"
#include "Clouds.h"
#include "ShaderObject.h"
#include "Utils.h"
#include "Vector3.h"
#include "Matrix4.h"

#include "ScreenAlignedTriangle.h"

#include <algorithm>

#include <stb_image.h>

const char* Clouds::transformFeedbackVaryings[] = { "vs_out_position", "vs_out_size_time_rand", "vs_out_depthviewspace" };
const unsigned int Clouds::maxNumCloudParticles = 4000;//16384;
const unsigned int Clouds::fourierOpacityMapSize = 512;
//const unsigned int Clouds::noiseTextureSize = 512;


Clouds::Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY, float farPlaneDistance) :
	screenResolutionX(screenResolutionX),
	screenResolutionY(screenResolutionY),
	farPlaneDistance(farPlaneDistance),
	moveShader(new ShaderObject("Shader\\cloudMove.vert", "", "", transformFeedbackVaryings, 3, false)),
	fomShader(new ShaderObject("Shader\\cloudPassThrough.vert", "Shader\\cloudFOM.frag", "Shader\\cloudFOM.geom")),
	fomFilterShader(new ShaderObject("Shader\\screenTri.vert", "Shader\\cloudFOMBlur.frag")),
	renderingShader(new ShaderObject("Shader\\cloudPassThrough.vert", "Shader\\cloudRendering.frag", "Shader\\cloudRendering.geom")),
	particleDepthBuffer(new float[maxNumCloudParticles]),
	particleIndexBuffer(new unsigned short[maxNumCloudParticles])
{
	shaderSetup();
	fboSetup();
	bufferSetup();
	samplerSetup();
	noiseSetup();
}

Clouds::~Clouds()
{
	glDeleteFramebuffers(1, &fourierOpacityMap_FBO[0]);
	glDeleteFramebuffers(1, &fourierOpacityMap_FBO[1]);
	glDeleteTextures(2, fourierOpacityMap_Textures[0]);
	glDeleteTextures(2, fourierOpacityMap_Textures[1]);
	glDeleteSamplers(1, &linearSampler_noMipMaps);
	glDeleteVertexArrays(1, &vao_cloudParticleBuffer_Read);
	glDeleteVertexArrays(1, &vao_cloudParticleBuffer_Write);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Read[0]);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Write[0]);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Read[1]);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Write[1]);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Read[2]);
	glDeleteBuffers(1, &vbo_cloudParticleBuffer_Write[2]);
	glDeleteBuffers(1, &ibo_cloudParticleRendering);
	glDeleteBuffers(1, &noiseTexture);
}

void Clouds::shaderSetup()
{
	// set ubo bindings, get uniform locations
		// move
	GLuint blockIndex = glGetUniformBlockIndex(moveShader->getProgram(), "View"); 
	glUniformBlockBinding(moveShader->getProgram(), blockIndex, 1);	// View binding=1
	blockIndex = glGetUniformBlockIndex(moveShader->getProgram(), "Timings"); 
	glUniformBlockBinding(moveShader->getProgram(), blockIndex, 2);	// Timings binding=2
	checkGLError("settings cloudMove");
		
		// fom
	fomShader->useProgram();
	fomShaderUniformIndex_cameraX = glGetUniformLocation(fomShader->getProgram(), "CameraRight");
	fomShaderUniformIndex_cameraY = glGetUniformLocation(fomShader->getProgram(), "CameraUp");
	fomShaderUniformIndex_cameraZ = glGetUniformLocation(fomShader->getProgram(), "CameraDir");
	fomShaderUniformIndex_LightDistancePlane_norm = glGetUniformLocation(fomShader->getProgram(), "LightDistancePlane_norm");
	fomShaderUniformIndex_lightViewProjection = glGetUniformLocation(fomShader->getProgram(), "LightViewProjection");
	glUniform1i(glGetUniformLocation(fomShader->getProgram(), "NoiseTexture"), 0);
	checkGLError("settings cloudFOM");

		// fom filter
	fomFilterShader->useProgram();
	glUniform1i(glGetUniformLocation(fomFilterShader->getProgram(), "FOMSampler0"), 1);
	glUniform1i(glGetUniformLocation(fomFilterShader->getProgram(), "FOMSampler1"), 2);
	fomFilterShaderUniformIndex_Offset = glGetUniformLocation(fomFilterShader->getProgram(), "Offset");
	checkGLError("settings fomFilterShader");

		// rendering
	renderingShader->useProgram();
	blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "Screen"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 0);	// Screen binding=0
	blockIndex = glGetUniformBlockIndex(renderingShader->getProgram(), "View"); 
	glUniformBlockBinding(renderingShader->getProgram(), blockIndex, 1);	// View binding=1
	renderingShaderUniformIndex_lightViewProjection = glGetUniformLocation(renderingShader->getProgram(), "LightViewProjection");
	renderingShaderUniformIndex_LightDistancePlane_norm = glGetUniformLocation(renderingShader->getProgram(), "LightDistancePlane_norm");
	renderingShaderUniformIndex_LightDirection = glGetUniformLocation(renderingShader->getProgram(), "LightDirection_viewspace");


	glUniform1i(glGetUniformLocation(renderingShader->getProgram(), "NoiseTexture"), 0);
	glUniform1i(glGetUniformLocation(renderingShader->getProgram(), "FOMSampler0"), 1);
	glUniform1i(glGetUniformLocation(renderingShader->getProgram(), "FOMSampler1"), 2);

	checkGLError("settings cloudRendering");
}

void Clouds::fboSetup()
{
	// Data Stuff
		// textures
	for(int i=0; i<2; ++i)
	{
		glGenTextures(2, fourierOpacityMap_Textures[i]);

		glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[i][0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fourierOpacityMapSize, fourierOpacityMapSize, 0, GL_RGBA, GL_FLOAT, 0);

		glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[i][1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fourierOpacityMapSize, fourierOpacityMapSize, 0, GL_RGBA, GL_FLOAT, 0);

		// fbo
		glGenFramebuffers(1, &fourierOpacityMap_FBO[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO[i]);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fourierOpacityMap_Textures[i][0], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fourierOpacityMap_Textures[i][1], 0);
			GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, DrawBuffers);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
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
}

void Clouds::bufferSetup()
{
	// data objects
		// init with random lifetimes as seed
	std::unique_ptr<float[]> startValues(new float[maxNumCloudParticles*3]);
	memset(startValues.get(), 0, sizeof(float) * maxNumCloudParticles * 3);

	glGenBuffers(3, vbo_cloudParticleBuffer_Read);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Read[0]);
	glBufferData(GL_ARRAY_BUFFER, maxNumCloudParticles * sizeof(float) * 3, 0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Read[1]);
	glBufferData(GL_ARRAY_BUFFER, maxNumCloudParticles * sizeof(float) * 3, startValues.get(), GL_STATIC_DRAW);	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Read[2]);
	glBufferData(GL_ARRAY_BUFFER, maxNumCloudParticles * sizeof(float) * 1, 0, GL_STATIC_READ);	// static read!
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkGLError("cloudVBO1");

	glGenBuffers(3, vbo_cloudParticleBuffer_Write);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write[0]);
	glBufferData(GL_ARRAY_BUFFER, maxNumCloudParticles * sizeof(float) * 3, 0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write[1]);
	glBufferData(GL_ARRAY_BUFFER, maxNumCloudParticles * sizeof(float) * 3, startValues.get(), GL_STATIC_DRAW);	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write[2]);
	glBufferData(GL_ARRAY_BUFFER, maxNumCloudParticles * sizeof(float) * 1, 0, GL_STATIC_READ);	// static read!
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkGLError("cloudVBO1");

	// ibo
	for(unsigned short i=0; i<maxNumCloudParticles; ++i)
		particleIndexBuffer[i] = i;
	glGenBuffers(1, &ibo_cloudParticleRendering);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cloudParticleRendering);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*maxNumCloudParticles, particleIndexBuffer.get(), GL_DYNAMIC_DRAW);	// dynamic draw! often writen, often rendered, never read (http://wiki.delphigl.com/index.php/glBufferData)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// generate vao for cloud particles (2 for ping/pong)
	glGenVertexArrays(1, &vao_cloudParticleBuffer_Read);
	glBindVertexArray(vao_cloudParticleBuffer_Read);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Read[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(0));	// position
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Read[1]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(0));	// size/lifetime/rand
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Read[2]);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), BUFFER_OFFSET(0));	// depth

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	checkGLError("cloudVAO_Read");

	glGenVertexArrays(1, &vao_cloudParticleBuffer_Write);
	glBindVertexArray(vao_cloudParticleBuffer_Write);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(0));	// position
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write[1]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(0));	// size/lifetime/rand
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write[2]);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), BUFFER_OFFSET(0));	// depth

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	checkGLError("cloudVAO_Write");
}

void Clouds::noiseSetup()
{
	/*glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	auto noise = PerlinNoiseGenerator::get().generate(noiseTextureSize,noiseTextureSize, 0.006f, 0.3f, 0.2f, 5, 0.3f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, noiseTextureSize, noiseTextureSize, 0, GL_RED, GL_UNSIGNED_BYTE, noise.get());
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);*/

	int TexSizeX, TexSizeY;
	stbi_uc* TextureData = stbi_load("particle.png", &TexSizeX, &TexSizeY, NULL, 4);
	if(!TextureData)
		Log::get() << "Error while loading texture particle.png\n";

	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TexSizeX, TexSizeX, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(TextureData);
}

void Clouds::particleSorting()
{
	// read depth vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cloudParticleBuffer_Write[2]);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * maxNumCloudParticles, particleDepthBuffer.get());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// sort by depth
	std::sort(particleIndexBuffer.get(), particleIndexBuffer.get() + maxNumCloudParticles, 
				[&](size_t i, size_t j) { return particleDepthBuffer[i] > particleDepthBuffer[j]; });

	// count num visible (the cloudMove.vert wrote a large depth value for all particles left/right/up/down/behind frustum)
	unsigned int numParticlesInvalid = 0;
	while(numParticlesInvalid < maxNumCloudParticles && particleDepthBuffer[particleIndexBuffer[numParticlesInvalid]] > farPlaneDistance)
		++numParticlesInvalid;
	numParticlesRender = maxNumCloudParticles - numParticlesInvalid;

	// write ibo
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cloudParticleRendering);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned short) * numParticlesRender, particleIndexBuffer.get() + numParticlesInvalid);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::cout << "Num Particles rendered: " << numParticlesRender << " \r";
}

void Clouds::createLightMatrices(Matrix4& lightView, Matrix4& lightProjection, float& farPlaneDistance,
								const Matrix4& viewProj, 
								const Vector3& viewDir, const Vector3& cameraPos,
								const Vector3& lightDir)
{
	Vector3 lightUp(viewDir.x, -(lightDir.z*viewDir.z+lightDir.x*viewDir.x)/lightDir.y, viewDir.z);	// why does this work? imported code from a old project ...
	Matrix4 matLightCamera(Matrix4::camera(0.0f, -lightDir, lightUp));
	Matrix4 mViewProjInv_LightCamera(viewProj.invert() * matLightCamera);
	Vector3 edges[8];
	edges[0] = Vector3(-1.0f, -1.0f, -1.0f) * mViewProjInv_LightCamera;
	edges[1] = Vector3(1.0f, -1.0f, -1.0f) * mViewProjInv_LightCamera;
	edges[2] = Vector3(-1.0f, 1.0f, -1.0f) * mViewProjInv_LightCamera;
	edges[3] = Vector3(1.0f, 1.0f, -1.0f) * mViewProjInv_LightCamera;
	edges[4] = Vector3(-1.0f, -1.0f, 1.0f) * mViewProjInv_LightCamera;
	edges[5] = Vector3(1.0f, -1.0f, 1.0f) * mViewProjInv_LightCamera;
	edges[6] = Vector3(-1.0f, 1.0f, 1.0f) * mViewProjInv_LightCamera;
	edges[7] = Vector3(1.0f, 1.0f, 1.0f) * mViewProjInv_LightCamera;

	// create box
	Vector3 min(999999);
	Vector3 max(-999999);
	for(int i=0; i<8; i++) 
	{
		min.x = std::min(edges[i].x, min.x);
		min.y = std::min(edges[i].y, min.y);
		min.z = std::min(edges[i].z, min.z);
		max.x = std::max(edges[i].x, max.x);
		max.y = std::max(edges[i].y, max.y);
		max.z = std::max(edges[i].z, max.z);
	}

	// find good camera position
	farPlaneDistance = max.z - min.z;
	Vector3 lightPos = Vector3((min.x + max.x)*0.5f, (min.y + max.y)*0.5f, min.z) * matLightCamera.invert();
	lightView = Matrix4::camera(lightPos, lightPos+lightDir, lightUp);
	// ortho
	lightProjection = Matrix4::projectionOrthogonal(max.x - min.x, max.y - min.y, 0, farPlaneDistance);
}

void Clouds::display(float timeSinceLastFrame, const Matrix4& viewMatrix, const Matrix4& viewProjection, const Vector3& cameraDirection, const Vector3& cameraPosition,
							ScreenAlignedTriangle& screenTri)
{
	glDisable(GL_DEPTH_TEST); 

	// through all the following passes the "read" buffer will be read

	// in the "write" buffer comes new data, that will be sorted
	// i'am hoping to maximes parallism this way: first the data will be upated, but everyone uses the old
	// then (still rendering) the upated particles will be sorted - frame ended, buffer switch
	// (so new data is used with a delay!)
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glBindSampler(0, linearSampler_noMipMaps);

	// move clouds
	glBindVertexArray(vao_cloudParticleBuffer_Read);
	glEnable(GL_RASTERIZER_DISCARD); 
	moveShader->useProgram();
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_cloudParticleBuffer_Write[0]);	// specify targets
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vbo_cloudParticleBuffer_Write[1]);	
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, vbo_cloudParticleBuffer_Write[2]);	
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, maxNumCloudParticles);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

	// render FOM
	fomShader->useProgram();

	Vector3 lightDirection(1, -1, 0);
	lightDirection.normalize();

		// matrix setup
	float lightFarPlane;
	Matrix4 lightProjection, lightView;
	createLightMatrices(lightView, lightProjection, lightFarPlane, viewProjection, cameraDirection, cameraPosition, lightDirection);
	Matrix4 lightViewProjection = lightView * lightProjection;
	glUniform3fv(fomShaderUniformIndex_cameraX, 1, Vector3(lightView.m11, lightView.m21, lightView.m31));
	glUniform3fv(fomShaderUniformIndex_cameraY, 1, Vector3(lightView.m12, lightView.m22, lightView.m32));
	glUniform3fv(fomShaderUniformIndex_cameraZ, 1, -Vector3(lightView.m13, lightView.m23, lightView.m33));
	glUniformMatrix4fv(fomShaderUniformIndex_lightViewProjection, 1, false, lightViewProjection);
	float lightDistancePlane_Norm[] = { -lightView.m13 / lightFarPlane, -lightView.m23 / lightFarPlane, -lightView.m33 / lightFarPlane, -lightView.m43 / lightFarPlane };
	glUniform4fv(fomShaderUniformIndex_LightDistancePlane_norm, 1, lightDistancePlane_Norm);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	// additive blending
	glViewport(0,0, fourierOpacityMapSize, fourierOpacityMapSize);

	glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO[0]);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_POINTS, 0, maxNumCloudParticles);
	
	glDisable(GL_BLEND);

	// new samplers
	glBindSampler(1, linearSampler_noMipMaps);
	glBindSampler(2, linearSampler_noMipMaps);

	// filter FOM
		// shader
	fomFilterShader->useProgram();
		// horizontal
	glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO[1]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0][0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0][1]);
	glUniform4f(fomFilterShaderUniformIndex_Offset, 1.5f / fourierOpacityMapSize, 0, -1.5f / fourierOpacityMapSize, 0);
	screenTri.display();
		// vertical
	glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[1][0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[1][1]);
	glUniform4f(fomFilterShaderUniformIndex_Offset, 0.0f, 1.5f / fourierOpacityMapSize, 0.0f, -1.5f / fourierOpacityMapSize);
	screenTri.display();

	// reset to backbuffer rendering
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0, screenResolutionX, screenResolutionY); 
	
	// render clouds
	glBindVertexArray(vao_cloudParticleBuffer_Read);
	renderingShader->useProgram();
	glUniformMatrix4fv(renderingShaderUniformIndex_lightViewProjection, 1, false, lightViewProjection);
	glUniform4fv(renderingShaderUniformIndex_LightDistancePlane_norm, 1, lightDistancePlane_Norm);
	glUniform3fv(renderingShaderUniformIndex_LightDirection, 1, (-lightDirection).transformNormal(viewMatrix));

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0][0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0][1]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cloudParticleRendering);
	
	glDrawElements(GL_POINTS, numParticlesRender, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// sort the new hopefully ready particles, while hopefully the screen itself is drawing
	particleSorting();
	// rotate read/write buffer
	swap(vao_cloudParticleBuffer_Read, vao_cloudParticleBuffer_Write);
	swap(vbo_cloudParticleBuffer_Read[0], vbo_cloudParticleBuffer_Write[0]);
	swap(vbo_cloudParticleBuffer_Read[1], vbo_cloudParticleBuffer_Write[1]);
	swap(vbo_cloudParticleBuffer_Read[2], vbo_cloudParticleBuffer_Write[2]);
}