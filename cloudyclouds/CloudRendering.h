#pragma once

#include <memory>

class CloudRendering
{
public:
	CloudRendering();
	~CloudRendering();

private:
	std::unique_ptr<class ShaderObject> cloudRendering;
};

