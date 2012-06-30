#pragma once

#include <memory>

class Matrix4;
class Vector3;

// references
// http://dl.acm.org/citation.cfm?id=1730831

class Clouds
{
public:
	Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY, float farPlaneDistance);
	~Clouds();

	void display(float timeSinceLastFrame, const Matrix4& viewProjection, const Vector3& cameraDirection, const Vector3& cameraPosition);

private:
	void shaderSetup();
	void fboSetup();
	void samplerSetup();
	void bufferSetup();
	void noiseSetup();

	void particleSorting();

	void createLightMatrices(Matrix4& viewMatrix, Matrix4& projectionMatrix, float& farPlaneDistance,
								const Matrix4& viewProj, 
								const Vector3& viewDir, const Vector3& cameraPos,
								const Vector3& lightDir);


	std::unique_ptr<class ShaderObject> moveShader;
	std::unique_ptr<class ShaderObject> fomShader;
	std::unique_ptr<class ShaderObject> renderingShader;

	unsigned int screenResolutionX, screenResolutionY;
	float farPlaneDistance;

	// fom shader uniform indices
	GLuint fomShaderUniformIndex_cameraX;
	GLuint fomShaderUniformIndex_cameraY;
	GLuint fomShaderUniformIndex_cameraZ;
	GLuint fomShaderUniformIndex_lightViewProjection;
	GLuint fomShaderUniformIndex_LightDistancePlane_norm;
	// rendering shader unifrom indices
	GLuint renderingShaderUniformIndex_lightViewProjection;
	GLuint renderingShaderUniformIndex_LightDistancePlane_norm;

	// cloud particle data
	GLuint vbo_cloudParticleBuffer_Read[3];
	GLuint vbo_cloudParticleBuffer_Write[3];
	GLuint vao_cloudParticleBuffer_Read;
	GLuint vao_cloudParticleBuffer_Write;

	GLuint ibo_cloudParticleRendering;

	// struct-representation of a particle vertex
	struct ParticleVertex
	{
		float position[3];
		float size_time_rand[3];
		float depth;
	};

	// buffer for cpu write/read operations
	std::unique_ptr<float[]>			particleDepthBuffer;
	std::unique_ptr<unsigned short[]>	particleIndexBuffer;
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

