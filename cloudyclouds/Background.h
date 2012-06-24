#pragma once

class ScreenAlignedTriangle;
class ShaderObject;

// background rendering - a sky gradient
class Background
{
public:
	Background(ScreenAlignedTriangle& screenTriangle);
	~Background();

	void display();

private:
	ScreenAlignedTriangle& screenTriangle;
	std::unique_ptr<ShaderObject> backgroundShader;

	GLuint cubeMap;
};

