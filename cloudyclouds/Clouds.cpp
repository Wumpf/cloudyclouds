#include "stdafx.h"
#include "Clouds.h"
#include "ShaderObject.h"
#include "Utils.h"
#include "Vector3.h"
#include "Matrix4.h"

#include "ScreenAlignedTriangle.h"

#include <algorithm>

#include <stb_image.h>

const char* Clouds::transformFeedbackVaryings[] = { "vs_out_position", "vs_out_size_time_rand", "vs_out_depthclipspace" };
const unsigned int Clouds::maxNumCloudParticles = 15000;// 16384;
const unsigned int Clouds::fourierOpacityMapSize = 1024;
//const unsigned int Clouds::noiseTextureSize = 512;


const GLuint Clouds::drawBuffers_Two[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
const GLuint Clouds::drawBuffers_One[1] = { GL_COLOR_ATTACHMENT0 };

Clouds::Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY, float farPlaneDistance, ScreenAlignedTriangle& screenTri) :
	screenResolutionX(screenResolutionX),
	screenResolutionY(screenResolutionY),
	farPlaneDistance(farPlaneDistance),
	moveShader(new ShaderObject("Shader\\cloudMove.vert", "", "", transformFeedbackVaryings, 3, false)),
	fomShader(new ShaderObject("Shader\\cloudPassThrough.vert", "Shader\\cloudFOM.frag", "Shader\\cloudFOM.geom")),
	fomFilterShader(new ShaderObject("Shader\\screenTri.vert", "Shader\\cloudFOMBlur.frag")),
	renderingShader(new ShaderObject("Shader\\cloudPassThrough.vert", "Shader\\cloudRendering.frag", "Shader\\cloudRendering.geom")),
	particleDepthBuffer(new float[maxNumCloudParticles]),
	particleIndexBuffer(new unsigned short[maxNumCloudParticles]),
	screenTri(screenTri)
{
	shaderInit();
	fboInit();
	bufferInit();
	samplerInit();
	noiseInit();
}

Clouds::~Clouds()
{
	glDeleteFramebuffers(1, &fourierOpacityMap_FBO[0]);
	glDeleteFramebuffers(1, &fourierOpacityMap_FBO[1]);
	glDeleteTextures(2, fourierOpacityMap_Textures[0]);
	glDeleteTextures(2, fourierOpacityMap_Textures[1]);
	glDeleteSamplers(1, &linearSampler_noMipMaps);
	glDeleteSamplers(1, &linearSampler_MipMaps);
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

void Clouds::shaderInit()
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

void Clouds::fboInit()
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
			glDrawBuffers(2, drawBuffers_Two);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Clouds::samplerInit()
{
	// sampler
	glGenSamplers(1, &linearSampler_noMipMaps);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(linearSampler_noMipMaps, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenSamplers(1, &linearSampler_MipMaps);
	glSamplerParameteri(linearSampler_MipMaps, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(linearSampler_MipMaps, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler_MipMaps, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(linearSampler_MipMaps, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(linearSampler_MipMaps, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Clouds::bufferInit()
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

void Clouds::noiseInit()
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

void Clouds::moveClouds()
{
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

void Clouds::renderFOM(const Matrix4& inverseViewProjection, const Vector3& cameraDirection, const Vector3& cameraPosition, const Vector3& lightDir)
{	
	fomShader->useProgram();

	// noise tex
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);	// noise is always on 0; the FOM textures are on 1 and 2
	glBindSampler(0, linearSampler_MipMaps);

	// matrix setup
	createLightMatrices(lightView, lightProjection, lightFarPlane, inverseViewProjection.invert(), cameraDirection, cameraPosition, lightDir);
	lightViewProjection = lightView * lightProjection;

	glUniform3fv(fomShaderUniformIndex_cameraX, 1, Vector3(lightView.m11, lightView.m21, lightView.m31));
	glUniform3fv(fomShaderUniformIndex_cameraY, 1, Vector3(lightView.m12, lightView.m22, lightView.m32));
	glUniform3fv(fomShaderUniformIndex_cameraZ, 1, -Vector3(lightView.m13, lightView.m23, lightView.m33));
	glUniformMatrix4fv(fomShaderUniformIndex_lightViewProjection, 1, false, lightViewProjection);
	lightDistancePlane_Norm[0] = -lightView.m13 / lightFarPlane;
	lightDistancePlane_Norm[1] = -lightView.m23 / lightFarPlane;
	lightDistancePlane_Norm[2] = -lightView.m33 / lightFarPlane;
	lightDistancePlane_Norm[3] = -lightView.m43 / lightFarPlane;
	glUniform4fv(fomShaderUniformIndex_LightDistancePlane_norm, 1, lightDistancePlane_Norm);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	// additive blending
	glViewport(0,0, fourierOpacityMapSize, fourierOpacityMapSize);

	glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO[0]);
	glDrawBuffers(2, drawBuffers_Two);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_POINTS, 0, maxNumCloudParticles);
	
	glDisable(GL_BLEND);

	// samplerstates for fom-textures
	glBindSampler(1, linearSampler_noMipMaps);
	glBindSampler(2, linearSampler_noMipMaps);


	// filter FOM
		// shader
	fomFilterShader->useProgram();
		// horizontal
	glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO[1]);
	glDrawBuffers(2, drawBuffers_Two);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0][0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0][1]);
	glUniform4f(fomFilterShaderUniformIndex_Offset, 1.5f / fourierOpacityMapSize, 0, -1.5f / fourierOpacityMapSize, 0);
	screenTri.display();
		// vertical
	glBindFramebuffer(GL_FRAMEBUFFER, fourierOpacityMap_FBO[0]);
	glDrawBuffers(2, drawBuffers_Two);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[1][0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[1][1]);
	glUniform4f(fomFilterShaderUniformIndex_Offset, 0.0f, 1.5f / fourierOpacityMapSize, 0.0f, -1.5f / fourierOpacityMapSize);
	screenTri.display();
	
	// reset to backbuffer rendering
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffers(1, drawBuffers_One);
	glViewport(0,0, screenResolutionX, screenResolutionY); 	
}

void Clouds::renderClouds(const Vector3& lightDir, const Matrix4& viewMatrix)
{
// render clouds
	//renderClouds(lightDir, view);
	renderingShader->useProgram();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cloudParticleRendering);
	glBindVertexArray(vao_cloudParticleBuffer_Read);

	// light settings
	glUniformMatrix4fv(renderingShaderUniformIndex_lightViewProjection, 1, false, lightViewProjection);
	glUniform4fv(renderingShaderUniformIndex_LightDistancePlane_norm, 1, lightDistancePlane_Norm);
	glUniform3fv(renderingShaderUniformIndex_LightDirection, 1, (-lightDir).transformNormal(viewMatrix));	// need direction TO light

	glBindSampler(1, linearSampler_noMipMaps);
	glBindSampler(2, linearSampler_noMipMaps);


	// textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);	// noise is always on 0; the FOM textures are on 1 and 2
	glBindSampler(0, linearSampler_MipMaps);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0][0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fourierOpacityMap_Textures[0][1]);

	// blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cloudParticleRendering);
	glBindVertexArray(vao_cloudParticleBuffer_Read);

	glEnable(GL_DEPTH_TEST); 

	glDrawElements(GL_POINTS, numParticlesRender, GL_UNSIGNED_SHORT, 0);

	// reset stuff
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisable(GL_BLEND);

	// reset textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Clouds::display(float timeSinceLastFrame, const Matrix4& inverseViewProjection, const Matrix4& view,
						const Vector3& cameraDirection, const Vector3& cameraPosition, const Vector3& lightDir)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// through all the following passes the "read" buffer will be read

	// in the "write" buffer comes new data, that will be sorted
	// i'am hoping to maximes parallism this way: first the data will be upated, but everyone uses the old
	// then (still rendering) the upated particles will be sorted - frame ended, buffer switch
	// (so new data is used with a delay!)
	
	// move clouds
	moveClouds();
	// render fom
	renderFOM(inverseViewProjection, cameraDirection, cameraPosition, lightDir);
	// render clouds
	renderClouds(lightDir, view);
	// sort the new hopefully ready particles, while hopefully the screen itself is drawing
	particleSorting();

	// rotate read/write buffer
	swap(vao_cloudParticleBuffer_Read, vao_cloudParticleBuffer_Write);
	swap(vbo_cloudParticleBuffer_Read[0], vbo_cloudParticleBuffer_Write[0]);
	swap(vbo_cloudParticleBuffer_Read[1], vbo_cloudParticleBuffer_Write[1]);
	swap(vbo_cloudParticleBuffer_Read[2], vbo_cloudParticleBuffer_Write[2]);
}