#pragma once

#include <memory>

class CloudRendering
{
public:
	CloudRendering();
	~CloudRendering();

	void display(float timeSinceLastFrame);

private:
	std::unique_ptr<class ShaderObject> cloudRendering;
};

