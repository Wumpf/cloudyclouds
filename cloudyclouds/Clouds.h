#pragma once

#include <memory>

class Clouds
{
public:
	Clouds(unsigned int screenResolutionX, unsigned int screenResolutionY);
	~Clouds();

	void display(float timeSinceLastFrame);

private:
	std::unique_ptr<class ShaderObject> renderingShader;

	// vbo with cloud particle data
	GLuint cloudParticleBuffer_Read;
	GLuint cloudParticleBuffer_Write;


	static const unsigned int maxNumCloudParticles;
};

