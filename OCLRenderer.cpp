#ifdef USE_DOUBLE
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include "OCLRenderer.hpp"
#include "CLUtils.hpp"

#ifdef __linux__

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#endif

OCLRenderer::OCLRenderer(size_t width, size_t height, size_t gpuNum, const std::string &kernelname,
                         const std::string &sourceFilename) : texture(Texture(width, height)), zoom(1.0f), pos({0.0f, 0.0f}), iterations(300)
{
	try
	{
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		size_t gpuCount = 0;
		if (platforms.size() == 0)
		{
			std::cerr << "[OCLRenderer] no opencl platforms available" << std::endl;
			exit(EXIT_FAILURE);
		}
		for (const auto &p : platforms)
		{
			// Select the platform with the same Vendor as the GL Context

			if (std::string(p.getInfo<CL_PLATFORM_VENDOR>()).find(std::string((const char *) glGetString(GL_VENDOR))) !=
			    std::string::npos)
			{
				// Get a list of devices on this platform
				std::vector<cl::Device> devices;
				p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
				for (const auto &d : devices)
				{
					// if the device is a gpu and has the support for sharing a gl context it is a suitable gpu
					if (d.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU &&
					    (std::string(d.getInfo<CL_DEVICE_EXTENSIONS>()).find(std::string("cl_khr_gl_sharing")) != std::string::npos))
					{
						if (gpuNum == gpuCount)
						{
#ifdef __linux__
							cl_context_properties properties[] = {
									CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
									CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
									CL_CONTEXT_PLATFORM, (cl_context_properties) (p)(),
									0
							};
#elif defined(_WIN32)
							cl_context_properties properties[] = {
									CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(),
									CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
									CL_CONTEXT_PLATFORM, (cl_context_properties) (mPlatforms[0])(),
									0
							};
#elif defined(__APPLE__)
							CGLContextObj glContext = CGLGetCurrentContext();
							CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);
							cl_context_properties properties[] = {
									CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
									(cl_context_properties) shareGroup,
							};
#endif
							device = d;
							context = cl::Context(device, properties);
							queue = cl::CommandQueue(context, device);
							// open and compile the program
							openProgram(sourceFilename, kernelname);
							// setup texture with the correct width and height
							reshape(width, height);
							return;
						}
						gpuCount++;
					}
				}
			}
		}

		std::cerr << "[OCLRenderer] requested gpu not or no gpu with GL context available" << std::endl;
		exit(EXIT_FAILURE);

	}
	catch (cl::Error error)
	{
		std::cout << error.what() << "(" << cl::errorString(error.err()) << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

bool OCLRenderer::openProgram(const std::string &filename, const std::string &kernelname)
{
	try
	{
		std::ifstream sourcefile(filename);
		std::string sourcecode(std::istreambuf_iterator<char>(sourcefile), (std::istreambuf_iterator<char>()));
		cl::Program::Sources source(1, std::make_pair(sourcecode.c_str(), sourcecode.length() + 1));

		// make program of the source code in the context
		program = cl::Program(context, source);

		// possibly some definitions for the kernel
		std::stringstream kerneloptions;

		// build program
		std::vector<cl::Device> tmpdevices;
		tmpdevices.push_back(device);
		program.build(tmpdevices, kerneloptions.str().c_str());

#ifdef USE_DOUBLE
		renderKernelFunc.reset(
				new cl::make_kernel<cl::ImageGL &, cl::Buffer &, cl::Buffer &, cl_float3, cl_int, cl_int, cl_int, cl_double, cl_double2, cl_int>(
						cl::Kernel(program, kernelname.c_str())));
#else
		renderKernelFunc.reset(
				new cl::make_kernel<cl::ImageGL &, cl::Buffer &, cl::Buffer &, cl_float3, cl_int, cl_int, cl_int, cl_float, cl_float2, cl_int>(
						cl::Kernel(program, "mandelbrot")));
#endif
	}
	catch (cl::Error error)
	{
		std::cout << error.what() << "(" << cl::errorString(error.err()) << ")" << std::endl;

		if (error.err() == CL_BUILD_PROGRAM_FAILURE)
			std::cout << "Build log:" << std::endl << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;

		exit(EXIT_FAILURE);
	}
}

void OCLRenderer::render(bool refresh)
{
	glFinish();
	try
	{
		queue.enqueueAcquireGLObjects(&glObjs);
		cl::EnqueueArgs eargs(queue, cl::NDRange(cl::nextDivisible(texture.width, 8),
		                                         cl::nextDivisible(texture.height, 8)), cl::NDRange(8, 8));
		sampleCount = refresh ? 1 : (sampleCount + 1);
#ifdef USE_DOUBLE
		(*renderKernelFunc)(eargs, imageBuffer, imageRawBuffer, randStatesBuffer, color, texture.width,
		                    texture.height, iterations, zoom, pos, sampleCount);
#else
		cl_float2 posf = {(cl_float) pos.s[0], (cl_float) pos.s[1]};
		(*renderKernelFunc)(eargs, imageBuffer, imageRawBuffer, randStatesBuffer, color, texture.width,
							texture.height, iterations, (cl_float) zoom, posf, sampleCount);
#endif
		queue.enqueueReleaseGLObjects(&glObjs);
		queue.finish();
	}
	catch (cl::Error error)
	{
		std::cout << error.what() << "(" << cl::errorString(error.err()) << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void OCLRenderer::reshape(size_t width, size_t height)
{
	texture.width = width;
	texture.height = height;
	texture.createEmptyTexture();
#ifdef CL_VERSION_1_2
	imageBuffer = cl::ImageGL(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture.id);
#else
	imageBuffer = cl::Image2DGL(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture.id);
#endif
	imageRawBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, width * height * sizeof(cl_float4));
	randStatesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, width * height * sizeof(cl_uint4));
	cl_uint *randStatesInitial = new cl_uint[4 * width * height];
	for (size_t i = 0; i < 4 * width * height; ++i)
		randStatesInitial[i] = i;
	queue.enqueueWriteBuffer(randStatesBuffer, CL_TRUE, 0, width * height * sizeof(cl_uint4), randStatesInitial);
	delete[] randStatesInitial;
	glObjs.clear();
	glObjs.push_back(imageBuffer);
}

const Texture &OCLRenderer::getTexture() const
{
	return texture;
}

void OCLRenderer::printAllDevices()
{

	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	size_t gpuCount = 0;
	for (size_t i = 0; i < platforms.size(); ++i)
	{
		std::cout << "[OCLRenderer] OpenCL Platform " << i << ": " << platforms[i].getInfo<CL_PLATFORM_VENDOR>() <<
		std::endl;

		// Get the list of devices available on the platform
		std::vector<cl::Device> devices;
		platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);

		for (size_t j = 0; j < devices.size(); ++j)
		{
			std::cout << "[OCLRenderer]   OpenCL device " << j << ": " << devices[j].getInfo<CL_DEVICE_NAME>() <<
			std::endl;
			std::cout << "[OCLRenderer]     Type: " << cl::deviceTypeString(devices[j].getInfo<CL_DEVICE_TYPE>()) <<
			std::endl;
			std::cout << "[OCLRenderer]     Units: " << devices[j].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
			std::cout << "[OCLRenderer]     Global memory: " <<
			devices[j].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / 1024 << "Kbytes" << std::endl;
			std::cout << "[OCLRenderer]     Local memory: " << devices[j].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / 1024 <<
			"Kbytes" << std::endl;
			std::cout << "[OCLRenderer]     Local memory type: " <<
			cl::memoryTypeString(devices[j].getInfo<CL_DEVICE_LOCAL_MEM_TYPE>()) << std::endl;
			std::cout << "[OCLRenderer]     Constant memory: " <<
			devices[j].getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>() / 1024 << "Kbytes" << std::endl;
			std::cout << "[OCLRenderer]     Device extensions: " <<
			devices[j].getInfo<CL_DEVICE_EXTENSIONS>() << std::endl;
			if (devices[j].getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU)
				std::cout << "[OCLRenderer]     GPU number: " << gpuCount++ << std::endl;
		}
	}
}

double OCLRenderer::getZoom() const
{
	return zoom;
}

void OCLRenderer::setZoom(double zoom)
{
	OCLRenderer::zoom = zoom;
}

const cl_double2 &OCLRenderer::getPos() const
{
	return pos;
}

void OCLRenderer::setPos(double x, double y)
{
	pos = {x, y};
}



const cl_float3 &OCLRenderer::getColor() const
{
	return color;
}

void OCLRenderer::setColor(const cl_float3 &color)
{
	OCLRenderer::color = color;
}

int OCLRenderer::getSampleCount() const
{
	return sampleCount;
}

cl_int OCLRenderer::getIterations() const
{
	return iterations;
}

void OCLRenderer::setIterations(cl_int iterations)
{
	OCLRenderer::iterations = iterations;
}

std::shared_ptr<std::vector<cl_float>> OCLRenderer::getImage() const
{
	std::shared_ptr<std::vector<cl_float>> retVal(new std::vector<cl_float>(texture.width * texture.height * 4));
	glFinish();
	queue.enqueueReadBuffer(imageRawBuffer, CL_TRUE, 0, texture.width * texture.height * sizeof(cl_float4),
	                        &((*retVal)[0]));
	queue.finish();
	for (size_t i = 0; i < texture.width * texture.height * 4; ++i)
		(*retVal)[i] /= sampleCount;
	return retVal;
}

