#pragma once

#include <memory>

class Clouds
{
public:
	Clouds();
	~Clouds();

	void display(float timeSinceLastFrame);

private:
	std::unique_ptr<class ShaderObject> renderingShader;
};

