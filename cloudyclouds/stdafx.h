// precompiled header

#ifdef WIN32
	#include <Windows.h>
#endif

#define GLEW_STATIC
#include <GL\glew.h>
#define GLFW_NO_GLU
#include <GL\glfw.h>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <math.h>

#include "Log.h"