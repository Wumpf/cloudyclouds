#include "stdafx.h"
#include "CloudRendering.h"
#include "ShaderObject.h"


CloudRendering::CloudRendering() :
	cloudRendering(new ShaderObject("","",""))
{
}


CloudRendering::~CloudRendering(void)
{
}
