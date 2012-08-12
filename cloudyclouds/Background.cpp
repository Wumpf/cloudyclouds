#include "stdafx.h"
#include "Background.h"
#include "ShaderObject.h"
#include "ScreenAlignedTriangle.h"
#include <stb_image.h>
#include "Vector3.h"
#include "Matrix4.h"

Background::Background(ScreenAlignedTriangle& screenTriangle) :
	screenTriangle(screenTriangle),
	backgroundShader(new ShaderObject("shader\\screenTri.vert", "shader\\background.frag"))
{
	GLuint blockIndex = glGetUniformBlockIndex(backgroundShader->getProgram(), "View"); 
	glUniformBlockBinding(backgroundShader->getProgram(), blockIndex, 1);	// View binding=1

	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "Heightmap"), 0);
	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "FOMSampler0"), 1);
	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "FOMSampler1"), 2);

	int TexSizeX, TexSizeY;
	stbi_uc* TextureData = stbi_load("heightmap.png", &TexSizeX, &TexSizeY, NULL, 4);
	if(!TextureData)
		Log::get() << "Error while loading texture heightmap.png\n";

	glGenTextures(1, &heightmapTexture);
	glBindTexture(GL_TEXTURE_2D, heightmapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TexSizeX, TexSizeX, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(TextureData);

	glGenSamplers(1, &samplerHeightmap);
	glSamplerParameteri(samplerHeightmap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(samplerHeightmap, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(samplerHeightmap, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerHeightmap, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Background::~Background()
{
	glDeleteTextures(1, &heightmapTexture);
	glDeleteSamplers(1, &samplerHeightmap);
}

void Background::display(const Vector3& lightDirection, Matrix4& lightViewProjection, float* lightDistancePlane_norm, GLuint FOMSampler0, GLuint FOMSampler1, GLuint FOMSamplerObject)
{
	backgroundShader->useProgram();
	glUniform3fv(glGetUniformLocation(backgroundShader->getProgram(), "LightDirection"), 1, -lightDirection);

	glUniformMatrix4fv(glGetUniformLocation(backgroundShader->getProgram(), "LightViewProjection"), 1, false, lightViewProjection);
	glUniform4fv(glGetUniformLocation(backgroundShader->getProgram(), "LightDistancePlane_norm"), 1, lightDistancePlane_norm);





	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FOMSampler0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, FOMSampler1);


	glBindSampler(0, samplerHeightmap);
	glBindSampler(1, FOMSamplerObject);
	glBindSampler(2, FOMSamplerObject);
	
	screenTriangle.display();

	glBindSampler(0, 0);
	glBindSampler(1, 0);
	glBindSampler(2, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
}
