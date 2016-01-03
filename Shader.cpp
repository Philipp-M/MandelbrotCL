#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include "Shader.hpp"

Shader::Shader(
		const std::string &name,
		const std::string &filename,
		ShaderType type
) : name(name), type(type)
{
	switch (type)
	{
		case VERTEX :
		{
			shaderId = glCreateShader(GL_VERTEX_SHADER_ARB);
			break;
		}
		case FRAGMENT :
		{
			shaderId = glCreateShader(GL_FRAGMENT_SHADER_ARB);
			break;
		}
	    case GEOMETRY :
	    {
		    shaderId = glCreateShader(GL_GEOMETRY_SHADER);
		    break;
	    }
		case COMPUTE:
		{
			shaderId = glCreateShader(GL_COMPUTE_SHADER);
			break;
		}
	}
	load(filename);
}

GLuint Shader::load(const std::string &filename)
{
	std::ifstream shaderSourceFileHandle(filename.c_str());
	if (!shaderSourceFileHandle.is_open())
	{
		std::cerr << "File not found " << filename.c_str() << "\n";
		exit(EXIT_FAILURE);
	}
	std::string source = std::string((std::istreambuf_iterator<char>(shaderSourceFileHandle)), std::istreambuf_iterator<char>());
	shaderSourceFileHandle.close();

	const char *data = source.c_str();
	glShaderSource(shaderId, 1, &data, NULL);
	glCompileShader(shaderId);
	GLint success = 0;
	GLchar infoLog[1024];
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderId, 1024, NULL, infoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", type, infoLog);
		exit(1);
	}
	return shaderId;
}

ShaderType const Shader::getType() const
{
	return type;
}

GLuint Shader::getShaderId() const
{
	return shaderId;
}

void Shader::setName(std::string &name)
{
	Shader::name = name;
}

const std::string &Shader::getName() const
{
	return name;
}

