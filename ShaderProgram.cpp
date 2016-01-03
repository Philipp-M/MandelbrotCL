#include <glm/ext.hpp>
#include <GL/glew.h>
#include <iostream>
#include <map>
#include "ShaderProgram.hpp"


ShaderProgram::ShaderProgram(const std::string &name) : name(name)
{
	shaderProgramId = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
	unbind();
	glDeleteProgram(shaderProgramId);
}

void ShaderProgram::attachShader(const Shader &shader)
{
	glAttachShader(shaderProgramId, shader.getShaderId());
	shaders.push_back(shader);
}

void ShaderProgram::link()
{
	GLint success = 0;
	GLchar errorLog[1024];

	/* Link shader code into executable shader program */
	glLinkProgram(shaderProgramId);

	/* Check results of linking step */
	glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success);

	if (success == 0)
	{
		glGetProgramInfoLog(shaderProgramId, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Error linking shader program: " << errorLog << std::endl;
		exit(1);
	}

	/* Check if shader program can be executed */
	glValidateProgram(shaderProgramId);
	glGetProgramiv(shaderProgramId, GL_VALIDATE_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(shaderProgramId, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Invalid shader program: " << errorLog << std::endl;
		exit(1);
	}

	/* Put linked shader program into drawing pipeline */
	glUseProgram(shaderProgramId);

}

ShaderProgram::ShaderProgram(const std::string &name, const Shader &vertex, const Shader &fragment) : name(name)
{
	shaderProgramId = glCreateProgram();
	attachShader(vertex);
	attachShader(fragment);
	link();
}

void ShaderProgram::bind() const
{
	glUseProgram(shaderProgramId);
}

void ShaderProgram::unbind() const
{
	glUseProgram(0);
}

bool ShaderProgram::vertexAttribPointer(const std::string &attribName, GLint size, GLenum type, GLsizei stride, const GLvoid *data,
                                        bool normalize) const
{
	glVertexAttribPointer(attributeLocation(attribName), size, type, normalize, stride, data);
}

GLuint ShaderProgram::attributeLocation(const std::string &name) const
{
	GLint loc = glGetAttribLocation(shaderProgramId, name.c_str());
	if (loc == -1)
	{
		std::cerr << "Attribute \"" << name << "\" not found in Program \"" << this->name << "\"\n";
		exit(EXIT_FAILURE);
	}
	return (GLuint) loc;
}

GLuint ShaderProgram::uniformLocation(const std::string &name) const
{
	GLint loc = glGetUniformLocation(shaderProgramId, name.c_str());
	if (loc == -1)
	{
		std::cerr << "Uniform \"" << name << "\" not found in Program \"" << this->name << "\"\n";
		exit(EXIT_FAILURE);
	}

	return (GLuint) loc;
}

GLuint ShaderProgram::bufferLocation(const std::string &name) const
{
	GLint loc = glGetProgramResourceIndex(shaderProgramId, GL_SHADER_STORAGE_BLOCK, name.c_str());
	if (loc == -1)
	{
		std::cerr << "Buffer \"" << name << "\" not found in Program \"" << this->name << "\"\n";
		exit(EXIT_FAILURE);
	}

	return (GLuint) loc;

	return (GLuint) loc;
}

void ShaderProgram::setBuffer(const std::string &name, GLint v) const
{
	glShaderStorageBlockBinding(shaderProgramId, bufferLocation(name), v);
}


void ShaderProgram::setUniform1i(const std::string &name, GLint v) const
{
	glUniform1i(uniformLocation(name), v);
}

void ShaderProgram::setUniform1f(const std::string &name, GLfloat v) const
{
	glUniform1f(uniformLocation(name), v);
}

void ShaderProgram::setUniform2f(const std::string &name, GLfloat v0, GLfloat v1) const
{
	glUniform2f(uniformLocation(name), v0, v1);
}

void ShaderProgram::setUniform2f(const std::string &name, glm::vec2 v) const
{
	glUniform2f(uniformLocation(name), v.x, v.y);
}

void ShaderProgram::setUniform3f(const std::string &name, GLfloat v0, GLfloat v1, GLfloat v2) const
{
	glUniform3f(uniformLocation(name), v0, v1, v2);
}

void ShaderProgram::setUniform3f(const std::string &name, glm::vec3 v) const
{
	glUniform3f(uniformLocation(name), v.x, v.y, v.z);
}

void ShaderProgram::setUniform4f(const std::string &name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const
{
	glUniform3f(uniformLocation(name), v0, v1, v2);
}

void ShaderProgram::setUniform4f(const std::string &name, glm::vec4 v) const
{
	glUniform4f(uniformLocation(name), v.x, v.y, v.z, v.w);
}

void ShaderProgram::setMatrixUniform4f(const std::string &name, glm::mat4 m) const
{
	glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}

void ShaderProgram::setMatrixUniform3f(const std::string &name, glm::mat3 m) const
{
	glUniformMatrix3fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}

