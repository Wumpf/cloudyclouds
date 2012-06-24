#include "stdafx.h"
#include "Background.h"
#include "ShaderObject.h"
#include "ScreenAlignedTriangle.h"
#include <stb_image.h>

Background::Background(ScreenAlignedTriangle& screenTriangle) :
	screenTriangle(screenTriangle),
	backgroundShader(new ShaderObject("shader\\screenTri.vert", "shader\\background.frag"))
{
	GLuint blockIndex = glGetUniformBlockIndex(backgroundShader->getProgram(), "View"); 
	glUniformBlockBinding(backgroundShader->getProgram(), blockIndex, 1);	// View binding=1
	/*backgroundShader->useProgram();
	glUniform1i(glGetUniformLocation(backgroundShader->getProgram(), "cubeTexture"), 0);


	glGenTextures(1, &cubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	int TexSizeX, TexSizeY;
	stbi_uc* textureData = stbi_load("Textures\\positive_x.png", &TexSizeX, &TexSizeY, NULL, 4);
	if(!textureData) Log::get() << "Error while loading texture positive_x.png\n";
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB8, TexSizeX, TexSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	stbi_image_free(textureData);

	textureData = stbi_load("Textures\\positive_y.png", &TexSizeX, &TexSizeY, NULL, 4);
	if(!textureData) Log::get() << "Error while loading texture positive_y.png\n";
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB8, TexSizeX, TexSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	stbi_image_free(textureData);

	textureData = stbi_load("Textures\\positive_z.png", &TexSizeX, &TexSizeY, NULL, 4);
	if(!textureData) Log::get() << "Error while loading texture positive_z.png\n";
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB8, TexSizeX, TexSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	stbi_image_free(textureData);

	textureData = stbi_load("Textures\\negative_x.png", &TexSizeX, &TexSizeY, NULL, 4);
	if(!textureData) Log::get() << "Error while loading texture negative_x.png\n";
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB8, TexSizeX, TexSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	stbi_image_free(textureData);

	textureData = stbi_load("Textures\\negative_y.png", &TexSizeX, &TexSizeY, NULL, 4);
	if(!textureData) Log::get() << "Error while loading texture negative_y.png\n";
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB8, TexSizeX, TexSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	stbi_image_free(textureData);

	textureData = stbi_load("Textures\\negative_z.png", &TexSizeX, &TexSizeY, NULL, 4);
	if(!textureData) Log::get() << "Error while loading texture negative_z.png\n";
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB8, TexSizeX, TexSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	stbi_image_free(textureData);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);*/
}

Background::~Background()
{
	glDeleteTextures(1, &cubeMap);
}

void Background::display()
{
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
	backgroundShader->useProgram();
	screenTriangle.display();
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
