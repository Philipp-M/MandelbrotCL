#include <iostream>
#include <ctime>
#include <string>
#include <SDL2/SDL.h>
#include "ShaderProgram.hpp"
#include "OCLRenderer.hpp"
#include "GLMain.hpp"

#define PROGRAM_NAME "Mandelbrot CL"


const size_t WIDTH = 1280;
const size_t HEIGHT = 720;

int main(int argc, char *argv[])
{
	srand48((unsigned int) std::time(nullptr));
	SDL_Window *mainwindow;
	SDL_GLContext maincontext;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{ /* Initialize SDL's Video subsystem */
		std::cerr << "Unable to initialize SDL" << std::endl;
		return 1;
	}

	/* Request opengl context. */
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	/* Create our window centered at 512x512 resolution */
	mainwindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                              WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!mainwindow)
	{ /* Die if creation failed */
		std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	bool quit = false;
	bool needUpdate = true;
	double zSpeed = 1.25;
	double width = WIDTH;
	double height = HEIGHT;
	double posRelX = 0;
	double posRelY = 0;
	double posX = 0;
	double posY = 0;
	double oldPosY = 0;
	double oldPosX = 0;
	bool leftPressed = false;
	bool rightPressed = false;
	GLMain glMain(mainwindow, maincontext);
	glMain.getOclRenderer()->setZoom(4.0);
	glMain.getOclRenderer()->setColor({(cl_float) (drand48() * M_PI * 2.0), (cl_float) (drand48() * M_PI * 2.0),
	                                   (cl_float) (drand48() * M_PI * 2.0)});
	glMain.getOclRenderer()->setPos(-1.2 / 4.0 * WIDTH / HEIGHT, -1.2 / 4.0);
	SDL_Event event;


	while (!quit)
	{
		glMain.getOclRenderer()->render(needUpdate);
		needUpdate = false;
		glMain.display();
		SDL_GL_SwapWindow(mainwindow);

		while (SDL_PollEvent(&event))
		{
			cl_double2 pos = glMain.getOclRenderer()->getPos();
			switch (event.type)
			{
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						width = event.window.data1;
						height = event.window.data2;
						glMain.reshape(event.window.data1, event.window.data2);
					}

					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_c)
					{
						glMain.getOclRenderer()->setColor(
								{(cl_float) (drand48() * M_PI * 2.0), (cl_float) (drand48() * M_PI * 2.0),
								 (cl_float) (drand48() * M_PI * 2.0)});
						needUpdate = true;
					}
					if (event.key.keysym.sym == SDLK_p)
						glMain.saveRenderedImage();
					if (event.key.keysym.sym == SDLK_PLUS)
					{
						glMain.getOclRenderer()->setIterations((cl_int) (glMain.getOclRenderer()->getIterations() * 1.25 + 1));
						needUpdate = true;
					}
					if (event.key.keysym.sym == SDLK_MINUS)
					{
						glMain.getOclRenderer()->setIterations(std::max(1, (cl_int) (glMain.getOclRenderer()->getIterations() * 0.8 - 1)));
						needUpdate = true;
					}

				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED)
						leftPressed = true;
					if (event.button.button == SDL_BUTTON_RIGHT && event.button.state == SDL_PRESSED)
					{
						rightPressed = true;
						oldPosY = posY;
						oldPosX = posX;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_RELEASED)
						leftPressed = false;
					if (event.button.button == SDL_BUTTON_RIGHT && event.button.state == SDL_RELEASED)
						rightPressed = false;
					break;
				case SDL_MOUSEWHEEL:

					if (event.wheel.y < 0)
					{
						glMain.getOclRenderer()->setPos(pos.s[0] * 1.0 / zSpeed - (1.0 - 1.0 / zSpeed) * posX / width,
						                                pos.s[1] * 1.0 / zSpeed - (1.0 - 1.0 / zSpeed) * (height - posY) / width);
						glMain.getOclRenderer()->setZoom(glMain.getOclRenderer()->getZoom() * zSpeed);
					}
					else if (event.wheel.y > 0)
					{
						glMain.getOclRenderer()->setPos(pos.s[0] * zSpeed - (1.0 - zSpeed) * posX / width,
						                                pos.s[1] * zSpeed - (1.0 - zSpeed) * (height - posY) / width);
						glMain.getOclRenderer()->setZoom(glMain.getOclRenderer()->getZoom() / zSpeed);
					}
					needUpdate = true;
					break;
				case SDL_MOUSEMOTION:
					posRelX = event.motion.xrel;
					posRelY = event.motion.yrel;
					posX = event.motion.x;
					posY = event.motion.y;
					if (leftPressed)
					{
						glMain.getOclRenderer()->setPos(-posRelX / width + pos.s[0], posRelY / width + pos.s[1]);
						needUpdate = true;
					}
					if (rightPressed)
					{
						pos = glMain.getOclRenderer()->getPos();
						double zoomFact = 1.0f - posRelY * 0.02f;
						glMain.getOclRenderer()->setPos(pos.s[0] * zoomFact - (1.0 - zoomFact) * oldPosX / width,
						                                pos.s[1] * zoomFact - (1.0 - zoomFact) * (height - oldPosY) / width);
						glMain.getOclRenderer()->setZoom(glMain.getOclRenderer()->getZoom() / zoomFact);
						needUpdate = true;
					}
					break;
			}
		}
	}
	glMain.cleanup();
	SDL_GL_DeleteContext(maincontext);
	SDL_DestroyWindow(mainwindow);
	SDL_Quit();

	return 0;
}
