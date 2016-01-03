#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <sstream>
#include "GLMain.hpp"

GLMain::GLMain(SDL_Window *window, SDL_GLContext &context)
{
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	initialize(w, h, window, context);
}

GLMain::~GLMain()
{
	cleanup();
}

void GLMain::cleanup()
{
	glDeleteBuffers(1, &vbo);
	oclRenderer.reset();
}

void GLMain::initialize(size_t width, size_t height, SDL_Window *window, SDL_GLContext &context)
{
	/********** OpenGL Context and GLEW **********/
	context = SDL_GL_CreateContext(window);

	GLenum rev;
	glewExperimental = GL_TRUE;
	rev = glewInit();

	if (GLEW_OK != rev)
	{
		std::cerr << "Error: " << glewGetErrorString(rev) << std::endl;
		exit(1);
	}


	/********** Vsync **********/
	SDL_GL_SetSwapInterval(1);

	/********** OpenCL initialization **********/
#ifdef USE_DOUBLE
	oclRenderer.reset(new OCLRenderer(width, height, 0, "mandelbrot", "kernels/default_double.cl"));
#else
	oclRenderer.reset(new OCLRenderer(width, height, 0, "mandelbrot", "kernels/default.cl"));
#endif

	/********** setup shader **********/
	shaderProgram.reset(new ShaderProgram("default"));
	shaderProgram->attachShader(Shader("vertex", "shader/defaultVs.glsl", ShaderType::VERTEX));
	shaderProgram->attachShader(Shader("fragment", "shader/defaultFs.glsl", ShaderType::FRAGMENT));
	shaderProgram->link();
	shaderProgram->bind();

	/********** setup the drawing primitive **********/
	static const GLfloat vertex_positions[] =
			{
					-1.0f, -1.0f,
					1.0f, -1.0f,
					-1.0f, 1.0f,
					1.0f, 1.0f
			};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), vertex_positions, GL_STATIC_DRAW);
	shaderProgram->vertexAttribPointer("pos", 2, GL_FLOAT, 0, 0, false);
	glEnableVertexAttribArray(shaderProgram->attributeLocation("pos"));

	/********** Other GL related stuff **********/
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

}

void GLMain::reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	oclRenderer->reshape(width, height);
	oclRenderer->render(true);
}

void GLMain::display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shaderProgram->bind();
	shaderProgram->setUniform1i("srcTex", 0);
	glBindTexture(GL_TEXTURE_2D, oclRenderer->getTexture().id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLMain::saveRenderedImage()
{
	// save the rendered image
	glFinish();
	size_t width = oclRenderer->getTexture().width;
	size_t height = oclRenderer->getTexture().height;
	uint32_t *pixels = new uint32_t[width * height];
	auto rawImage = oclRenderer->getImage();

	// simple linear tone mapping
	for (size_t i = 0; i < width * height * 4; i += 4)
		pixels[i / 4] = (((uint32_t) std::min(0xFF, std::max(0x0, (int) ((*rawImage)[i + 0] * 255.0f)))) << 24) |
		                (((uint32_t) std::min(0xFF, std::max(0x0, (int) ((*rawImage)[i + 1] * 255.0f)))) << 16) |
		                (((uint32_t) std::min(0xFF, std::max(0x0, (int) ((*rawImage)[i + 2] * 255.0f)))) << 8) |
		                (((uint32_t) std::min(0xFF, std::max(0x0, (int) ((*rawImage)[i + 3] * 255.0f)))) << 0);

	SDL_Surface *image = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, 4 * width, 0xFF000000, 0x00FF0000,
	                                              0x0000FF00, 0x000000FF);
	std::ostringstream stringStream;
	stringStream << "render_" << time(nullptr) << "_" << oclRenderer->getSampleCount() << "SPP.bmp";
	SDL_SaveBMP(image, stringStream.str().c_str());
	SDL_FreeSurface(image);
	delete[] pixels;
}

OCLRenderer *GLMain::getOclRenderer()
{
	return oclRenderer.get();
}