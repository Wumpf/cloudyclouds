#pragma once

#include <memory>
#include "Matrix4.h"

class Vector3;

// references
// http://dl.acm.org/citation.cfm?id=1730831

class Clouds
{
public:
	Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY, float farPlaneDistance, class ScreenAlignedTriangle& screenTri);
	~Clouds();

	void display(float timeSinceLastFrame, const Matrix4& inverseViewProjection, const Matrix4& view,
				const Vector3& cameraDirection, const Vector3& cameraPosition, const Vector3& lightDir);

	float* getLightDistancePlane_Norm() { return lightDistancePlane_Norm; }
	Matrix4& getLightViewProjection() { return lightViewProjection; }
	GLuint getFOMTexture0() { return fourierOpacityMap_Textures[0][0]; }
	GLuint getFOMTexture1() { return fourierOpacityMap_Textures[0][1]; }
	GLuint getFOMSampler() { return linearSampler_noMipMaps; }

private:
	void shaderInit();
	void fboInit();
	void samplerInit();
	void bufferInit();
	void noiseInit();


	void moveClouds();
	void renderFOM(const Matrix4& inverseViewProjection, const Vector3& viewDir, const Vector3& cameraPos, const Vector3& lightDirection);
	void renderClouds(const Vector3& lightDir, const Matrix4& viewMatrix);
	void particleSorting();


	void createLightMatrices(Matrix4& lightView, Matrix4& lightProjection, float& farPlaneDistance,
								const Matrix4& viewProj, 
								const Vector3& viewDir, const Vector3& cameraPos,
								const Vector3& lightDir);
	/*
	void createLightMatrices(const Matrix4& inverseViewProjection, 
								const Vector3& cameraDirection, const Vector3& cameraPos,
								const Vector3& lightDir);
								*/
	// shader
	std::unique_ptr<class ShaderObject> moveShader;
	std::unique_ptr<class ShaderObject> fomShader;
	std::unique_ptr<class ShaderObject> fomFilterShader;
	std::unique_ptr<class ShaderObject> renderingShader;

	// screen infos
	unsigned int screenResolutionX, screenResolutionY;
	float farPlaneDistance;

	// light infos (updated every frame)
	float lightFarPlane;
	Matrix4 lightProjection;
	Matrix4 lightView;
	Matrix4 lightViewProjection;
	float lightDistancePlane_Norm[4];

	// fom shader uniform indices
	GLuint fomShaderUniformIndex_cameraX;
	GLuint fomShaderUniformIndex_cameraY;
	GLuint fomShaderUniformIndex_cameraZ;
	GLuint fomShaderUniformIndex_lightViewProjection;
	GLuint fomShaderUniformIndex_LightDistancePlane_norm;
	// fom filter uniform indices
	GLuint fomFilterShaderUniformIndex_Offset;
	// rendering shader unifrom indices
	GLuint renderingShaderUniformIndex_lightViewProjection;
	GLuint renderingShaderUniformIndex_LightDistancePlane_norm;
	GLuint renderingShaderUniformIndex_LightDirection;

	// cloud particle data
	GLuint vbo_cloudParticleBuffer_Read[3];
	GLuint vbo_cloudParticleBuffer_Write[3];
	GLuint vao_cloudParticleBuffer_Read;
	GLuint vao_cloudParticleBuffer_Write;
	GLuint ibo_cloudParticleRendering;

	// buffer for cpu write/read operations
	std::unique_ptr<float[]>			particleDepthBuffer;
	std::unique_ptr<unsigned short[]>	particleIndexBuffer;
	unsigned int numParticlesRender;

	// FOM - two for filtering
	GLuint fourierOpacityMap_Textures[2][2];
	GLuint fourierOpacityMap_FBO[2];

	// noise
	GLuint noiseTexture;

	// sampler
	GLuint linearSampler_noMipMaps;
	GLuint linearSampler_MipMaps;

	// screen tri
	ScreenAlignedTriangle& screenTri;

	// mrt settings
	static const GLuint drawBuffers_One[1];
	static const GLuint drawBuffers_Two[2];


	static const char* transformFeedbackVaryings[];
	static const unsigned int maxNumCloudParticles;
	static const unsigned int fourierOpacityMapSize;
	static const unsigned int noiseTextureSize;
};

