#include "stdafx.h"
#include "Utils.h"

#include "stb_image.h"

#define _USE_MATH_DEFINES
#include <math.h>

float degToRad(float degree)
{
	return static_cast<float>(degree * (M_PI / 360));
}

float radToDeg(float rad)
{
	return static_cast<float>(rad / (M_PI / 360));
}

bool checkGLError(const char* Title)
{
	int Error;
	if((Error = glGetError()) != GL_NO_ERROR)
	{
		std::string ErrorString;
		switch(Error)
		{
		case GL_INVALID_ENUM:
			ErrorString = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			ErrorString = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			ErrorString = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			ErrorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			ErrorString = "GL_OUT_OF_MEMORY";
			break;
		default:
			ErrorString = "UNKNOWN";
			break;
		}
		fprintf(stdout, "OpenGL Error(%s): %s\n", ErrorString.c_str(), Title);
	}
	return Error == GL_NO_ERROR;
}

float random(float min, float max)
{
	return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
}

GLuint loadTextureWithMipMaps(const std::string& textureFilename)
{
	int TexSizeX, TexSizeY;

	stbi_uc* TextureData = stbi_load(textureFilename.c_str(), &TexSizeX, &TexSizeY, NULL, 4);
	if(!TextureData)
	{
		Log::get() << "Error while loading texture \"" + textureFilename + "\"\n";
		return 0;
	}

	GLuint Texture;
	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TexSizeX, TexSizeX, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(TextureData);

	return Texture;
}