#include "stdafx.h"
#include "ShaderObject.h"


ShaderObject::ShaderObject(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename, const std::string& geometryShaderFilename) :
	hasGeometryShader(geometryShaderFilename != ""),
	hasFragmentShader(fragmentShaderFilename != ""),
	hasVertexShader(vertexShaderFilename != ""),

	vertexShaderOrigin(vertexShaderFilename),
	fragmentShaderOrigin(fragmentShaderFilename),
	geometryShaderOrigin(geometryShaderFilename)
{
	// vertex shader
	if(hasVertexShader)
	{
		vertexShader = glCreateShader(GL_VERTEX_SHADER);			// create object
		std::string shaderSource = readFile(vertexShaderFilename);	// read file
		const char* sourcePtr = shaderSource.c_str();		
		glShaderSource(vertexShader, 1, &sourcePtr, NULL);			// attach shader code
		glCompileShader(vertexShader);								// compile
		printShaderInfoLog(vertexShader, vertexShaderFilename);							// log output
	}

	// fragment shader
	if(hasFragmentShader)
	{
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);	// create object
		std::string shaderSource = readFile(fragmentShaderFilename);		// read file
		const char* sourcePtr = shaderSource.c_str();		
		glShaderSource(fragmentShader, 1, &sourcePtr, NULL);	// attach shader code
		glCompileShader(fragmentShader);						// compile
		printShaderInfoLog(fragmentShader, fragmentShaderFilename);						// log output
	}

	// geometry shader
	if(hasGeometryShader)
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);// create object
		std::string shaderSource = readFile(geometryShaderFilename);	// read file
		const char* sourcePtr = shaderSource.c_str();		
		glShaderSource(geometryShader, 1, &sourcePtr, NULL);// attach shader code
		glCompileShader(geometryShader);					// compile
		printShaderInfoLog(geometryShader, geometryShaderFilename);					// log output
	}

	// Create shader program
	program = glCreateProgram();	

	// Attach shader
	if(hasVertexShader)
		glAttachShader(program, vertexShader);
	if(hasFragmentShader)
		glAttachShader(program, fragmentShader);
	if(hasGeometryShader)
		glAttachShader(program, geometryShader);

	// Link program
	glLinkProgram(program);
	printProgramInfoLog(program);
}


ShaderObject::~ShaderObject()
{
	glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
	if(hasGeometryShader)
		glDetachShader(program, geometryShader);
    glDeleteProgram(program);
}

std::string ShaderObject::readFile(std::string fileName)
{
	std::string fileContent;
	std::string line;

	std::ifstream file(fileName.c_str());
	if (file.is_open())
	{
		while (!file.eof())
		{
			getline (file,line);
			line += "\n";
			fileContent += line;					
		}
		file.close();
	}
	else
		Log::get() << "ERROR: Unable to open file " << fileName << "\n";

	return fileContent;
}

void ShaderObject::printShaderInfoLog(GLuint shader, const std::string& shaderName)
{
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);		
	infoLog = (char *)malloc(infologLength);
	glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
	Log::get() << shaderName << ":\n" << infoLog;
	free(infoLog);
}

// Print information about the linking step
void ShaderObject::printProgramInfoLog(GLuint program)
{
	GLint infoLogLength = 0;
	GLsizei charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH,&infoLogLength);
	infoLog = (char *)malloc(infoLogLength);
	glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog);
	Log::get() << infoLog;
	free(infoLog);
}
