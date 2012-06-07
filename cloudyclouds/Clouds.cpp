#include "stdafx.h"
#include "Clouds.h"
#include "ShaderObject.h"


Clouds::Clouds() :
	renderingShader(new ShaderObject("Shader\\cloudRendering.vert", "Shader\\cloudRendering.frag", "Shader\\cloudRendering.geom"))
{
}

Clouds::~Clouds()
{
}

void Clouds::display(float timeSinceLastFrame)
{
}