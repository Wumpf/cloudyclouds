#pragma once

#include <memory>

// references
// http://dl.acm.org/citation.cfm?id=1730831

class Clouds
{
public:
	Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY, float farPlaneDistance);
	~Clouds();

	void display(float timeSinceLastFrame);

private:
	void shaderSetup();
	void fboSetup();
	void samplerSetup();
	void bufferSetup();
	void noiseSetup();

	void particleSorting();


	std::unique_ptr<class ShaderObject> moveShader;
	std::unique_ptr<class ShaderObject> fomShader;
	std::unique_ptr<class ShaderObject> renderingShader;

	unsigned int screenResolutionX, screenResolutionY;
	float farPlaneDistance;

	// fom shader uniform indices
	GLuint fomShaderUniformIndex_cameraX;
	GLuint fomShaderUniformIndex_cameraY;
	GLuint fomShaderUniformIndex_lightViewProjection;
	GLuint fomShaderUniformIndex_lightViewMatrix;
	GLuint fomShaderUniformIndex_farPlane;
	// rendering shader unifrom indices
	GLuint renderingShaderUniformIndex_lightViewProjection;
	GLuint renderingShaderUniformIndex_lightView;
	GLuint renderingShaderUniformIndex_lightFarPlane;


	// cloud particle data
	GLuint vbo_cloudParticleBuffer_Read;
	GLuint vbo_cloudParticleBuffer_Write;
	GLuint vao_cloudParticleBuffer_Read;
	GLuint vao_cloudParticleBuffer_Write;

	GLuint ibo_cloudParticleRendering;

	// struct-representation of a particle vertex
	struct ParticleVertex
	{
		float position[3];
		float size;
		float lifetime;
		float depth;
	};

	// buffer for cpu write/read operations
	std::unique_ptr<ParticleVertex[]> particleVertexBuffer;
	std::unique_ptr<unsigned short[]> particleIndexBuffer;
	unsigned int numParticlesRender;

	// FOM
	GLuint fourierOpacityMap_Textures[2];
	GLuint fourierOpacityMap_DepthBuffer;
	GLuint fourierOpacityMap_FBO;

	// noise
	GLuint noiseTexture;

	// sampler
	GLuint linearSampler_noMipMaps;

	static const char* transformFeedbackVaryings[];
	static const unsigned int maxNumCloudParticles;
	static const unsigned int fourierOpacityMapSize;
	static const unsigned int noiseTextureSize;
};

