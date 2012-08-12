#pragma once

class ScreenAlignedTriangle;
class ShaderObject;

// background rendering - a sky gradient
class Background
{
public:
	Background(ScreenAlignedTriangle& screenTriangle);
	~Background();

	void display(const class Vector3& lightDirection, class Matrix4& LightViewProjection, float* lightDistancePlane_norm, GLuint FOMSampler0, GLuint FOMSampler1, GLuint FOMSamplerObject);

private:
	ScreenAlignedTriangle& screenTriangle;
	std::unique_ptr<ShaderObject> backgroundShader;
	GLuint heightmapTexture;
	GLuint samplerHeightmap;
	GLuint rockTexture;
	GLuint grassTexture;
};

