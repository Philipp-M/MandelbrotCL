#pragma once

#include <string>
#include <cstddef>
#include <GL/gl.h>

struct Texture
{
	size_t width;
	size_t height;
	GLuint id;

	Texture(size_t width, size_t height) : width(width), height(height), id(0xFFFFFFFF)
	{ createEmptyTexture(); }

	~Texture()
	{ glDeleteTextures(1, &id); }

	// TODO
	Texture(size_t width, size_t height, std::string filename) : width(width), height(height)
	{ }

	void createEmptyTexture()
	{
		glDeleteTextures(1, &id);
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, id);
	}

};