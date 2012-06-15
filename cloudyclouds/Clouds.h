#pragma once

#include <memory>

class Clouds
{
public:
	Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY);
	~Clouds();

	void display(float timeSinceLastFrame);

private:
	std::unique_ptr<class ShaderObject> moveShader;
	std::unique_ptr<class ShaderObject> renderingShader;

	// cloud particle data
	GLuint vbo_cloudParticleBuffer_Read;
	GLuint vbo_cloudParticleBuffer_Write;
	GLuint vao_cloudParticleBuffer_Read;
	GLuint vao_cloudParticleBuffer_Write;

	static const char* transformFeedbackVaryings[];
	static const unsigned int maxNumCloudParticles;
};

