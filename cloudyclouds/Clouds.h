#pragma once

#include <memory>

// references
// http://dl.acm.org/citation.cfm?id=1730831

class Clouds
{
public:
	Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY);
	~Clouds();

	void display(float timeSinceLastFrame);

private:
	std::unique_ptr<class ShaderObject> moveShader;
	std::unique_ptr<class ShaderObject> fomShader;
	std::unique_ptr<class ShaderObject> renderingShader;

	unsigned int screenResolutionX, screenResolutionY;

	// fom shader uniform indices
	GLuint fomShaderUniformIndex_cameraX;
	GLuint fomShaderUniformIndex_cameraY;
	GLuint fomShaderUniformIndex_lightViewProjection;

	// cloud particle data
	GLuint vbo_cloudParticleBuffer_Read;
	GLuint vbo_cloudParticleBuffer_Write;
	GLuint vao_cloudParticleBuffer_Read;
	GLuint vao_cloudParticleBuffer_Write;

	// FOM
	GLuint fourierOpacityMap_Textures[2];
	GLuint fourierOpacityMap_DepthBuffer;
	GLuint fourierOpacityMap_FBO;

	// sampler
	GLuint linearSampler_noMipMaps;

	static const char* transformFeedbackVaryings[];
	static const unsigned int maxNumCloudParticles;
	static const unsigned int fourierOpacityMapSize;
};

