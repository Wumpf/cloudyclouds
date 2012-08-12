#include "stdafx.h"
#include "Background.h"
#include "ShaderObject.h"
#include "ScreenAlignedTriangle.h"
#include <stb_image.h>
#include "Vector3.h"
#include "Matrix4.h"

GLuint loadTexture(const std::string& name)
{
	int TexSizeX, TexSizeY;
	stbi_uc* TextureData = stbi_load(name.c_str(), &TexSizeX, &TexSizeY, NULL, 4);
	if(!TextureData)
		Log::get() << "Error while loading texture heightmap.png\n";

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TexSizeX, TexSizeX, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(TextureData);

	return texture;
}

Background::Background(ScreenAlignedTriangle& screenTriangle) :
	screenTriangle(screenTriangle),
	backgroundShader(new ShaderObject("shader\\screenTri.vert", "shader\\background.frag"))
{
	backgroundShader->useProgram();
	GLuint blockIndex = glGetUniformBlockIndex(backgroundShader->getProgram(), "View"); 
	glUniformBlockBinding(backgroundShader->getProgram(), blockIndex, 1);	// View binding=1
	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "Heightmap"), 0);
	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "FOMSampler0"), 1);
	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "FOMSampler1"), 2);
	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "rockTexture"), 3);
	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "grassTexture"), 4);
	glUseProgram(0);


	heightmapTexture = loadTexture("heightmap.png");
	rockTexture = loadTexture("rock.bmp");
	grassTexture = loadTexture("grass.bmp");

	glGenSamplers(1, &samplerHeightmap);
	glSamplerParameteri(samplerHeightmap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(samplerHeightmap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplerHeightmap, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerHeightmap, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Background::~Background()
{
	glDeleteTextures(1, &heightmapTexture);
	glDeleteTextures(1, &rockTexture);
	glDeleteSamplers(1, &samplerHeightmap);
}

void Background::display(const Vector3& lightDirection, Matrix4& lightViewProjection, float* lightDistancePlane_norm, GLuint FOMSampler0, GLuint FOMSampler1, GLuint FOMSamplerObject)
{
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_DEPTH_TEST);

	backgroundShader->useProgram();
	glUniform3fv(glGetUniformLocation(backgroundShader->getProgram(), "LightDirection"), 1, -lightDirection);

	glUniformMatrix4fv(glGetUniformLocation(backgroundShader->getProgram(), "LightViewProjection"), 1, false, lightViewProjection);
	glUniform4fv(glGetUniformLocation(backgroundShader->getProgram(), "LightDistancePlane_norm"), 1, lightDistancePlane_norm);

	glBindSampler(0, samplerHeightmap);
	glBindSampler(1, FOMSamplerObject);
	glBindSampler(2, FOMSamplerObject);
	glBindSampler(3, samplerHeightmap);
	glBindSampler(4, samplerHeightmap);
	glBindSampler(5, samplerHeightmap);
	glBindSampler(6, samplerHeightmap);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FOMSampler0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, FOMSampler1);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, rockTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, grassTexture);

	screenTriangle.display();

	glBindSampler(0, 0);
	glBindSampler(1, 0);
	glBindSampler(2, 0);
	glBindSampler(3, 0);
	glBindSampler(4, 0);
	glBindSampler(5, 0);
	glBindSampler(6, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, 0);


	glDepthFunc(GL_LEQUAL);
}
