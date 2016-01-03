#pragma once


#include "ShaderProgram.hpp"
#include "OCLRenderer.hpp"
#include <SDL2/SDL.h>
#include <memory>

class GLMain
{
private:
	std::shared_ptr<ShaderProgram> shaderProgram;
	std::shared_ptr<OCLRenderer> oclRenderer;
	GLuint vao;
	GLuint vbo;

	/**
	 * initializes a basic opengl window and the opencl renderer
	 */
	void initialize(size_t width, size_t height, SDL_Window *window, SDL_GLContext &context);

public:
	GLMain(SDL_Window *window, SDL_GLContext &context);

	~GLMain();

	/**
	 * deletes all the acquired buffers and the opencl renderer
	 */
	void cleanup();

	/**
	 * displays the rendered texture
	 */
	void display();

	/**
	 * resizes the opengl screen and does the same to the opencl renderer
	 */
	void reshape(int width, int height);

	/**
	 * saves a screencapture in the current directory with the following name scheme:
	 * render_{CURRENT_TIME}_{SAMPLE_COUNT}_Spp.bmp
	 */
	void saveRenderedImage();

	/**
	 * reference to the opencl renderer
	 */
	OCLRenderer * getOclRenderer();
};

