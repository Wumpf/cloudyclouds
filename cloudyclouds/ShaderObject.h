#pragma once

// easy to use wrapper for opengl shader
class ShaderObject
{
public:
	ShaderObject(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename, const std::string& geometryShaderFilename = "");
	~ShaderObject();

	GLuint getProgram() { return program; }
	void useProgram()	{ glUseProgram(program); }

private:
	/// reads a file and returns the content as a string
	std::string readFile(std::string fileName);
	// Print information about the compiling step
	void ShaderObject::printShaderInfoLog(GLuint shader, const std::string& shaderName);
	// Print information about the linking step
	void ShaderObject::printProgramInfoLog(GLuint program);


	const bool hasGeometryShader;
	const bool hasFragmentShader;
	const bool hasVertexShader;

	const std::string vertexShaderOrigin;
	const std::string fragmentShaderOrigin;
	const std::string geometryShaderOrigin;

	GLuint program;
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint geometryShader;
};

