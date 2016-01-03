#pragma once

#include <string>
#include <vector>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "Shader.hpp"

class ShaderProgram
{
private:
public:
private:
	GLuint shaderProgramId;
	std::string name;
	std::vector<Shader> shaders;
public:
	ShaderProgram(const std::string &name = "default");

	ShaderProgram(const std::string &name, const Shader &vertex, const Shader &fragment);

	~ShaderProgram();

	void attachShader(const Shader &shader);

	void link();

	bool vertexAttribPointer(const std::string &attribName, GLint size, GLenum type, GLsizei stride, const GLvoid *data,
	                         bool normalize) const;

	GLuint attributeLocation(const std::string &name) const;

	GLuint uniformLocation(const std::string &name) const;

	GLuint bufferLocation(const std::string &name) const;

	void setBuffer(const std::string &name, GLint v) const;

	void setUniform1i(const std::string &name, GLint v) const;

	void setUniform1f(const std::string &name, GLfloat v) const;

	void setUniform2f(const std::string &name, GLfloat v0, GLfloat v1) const;

	void setUniform2f(const std::string &name, glm::vec2 v) const;

	void setUniform3f(const std::string &name, GLfloat v0, GLfloat v1, GLfloat v2) const;

	void setUniform3f(const std::string &name, glm::vec3 v) const;

	void setUniform4f(const std::string &name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const;

	void setUniform4f(const std::string &name, glm::vec4 v) const;

	void setMatrixUniform4f(const std::string &name, glm::mat4 m) const;

	void setMatrixUniform3f(const std::string &name, glm::mat3 m) const;

	void bind() const;

	void unbind() const;

	GLuint getId() const;

	const std::string &getName() const;

	void setName(const std::string &name);
};


inline GLuint ShaderProgram::getId() const
{
	return shaderProgramId;
}

inline const std::string &ShaderProgram::getName() const
{
	return name;
}

inline void ShaderProgram::setName(const std::string &name)
{
	ShaderProgram::name = name;
}
