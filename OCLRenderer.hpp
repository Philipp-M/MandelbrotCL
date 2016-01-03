#pragma once

#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <memory>
#include "Texture.hpp"

class OCLRenderer
{
private:
	//Mandelbrot specific
	cl_double zoom;
	cl_double2 pos;
	cl_float3 color;
	cl_int sampleCount;
	cl_int iterations;

	cl::Context context;
	cl::Device device;
	cl::Program program;
	cl::CommandQueue queue;
	cl::Buffer randStatesBuffer;
	cl::Buffer imageRawBuffer;
#ifdef USE_DOUBLE
	std::shared_ptr<cl::make_kernel<cl::ImageGL &, cl::Buffer &, cl::Buffer &, cl_float3, cl_int, cl_int, cl_int, cl_double, cl_double2, cl_int>> renderKernelFunc;
#else
	std::shared_ptr<cl::make_kernel<cl::ImageGL &, cl::Buffer&, cl::Buffer&, cl_float3, cl_int, cl_int, cl_int, cl_float, cl_float2, cl_int>> renderKernelFunc;
#endif
	std::vector<cl::Memory> glObjs;
#ifdef CL_VERSION_1_2
	cl::ImageGL imageBuffer;
#else
	cl::Image2DGL imageBuffer;
#endif
	Texture texture;

public:
	/**
	 * initializes opencl with the gpuNumth device that has a gl context
	 *
	 * @param width the width of the desired texture size
	 * @param height the height of the desired texture size
	 * @param gpuNum the gpu with a gl context that will be chosen should be 0 in almost every case
	 * @param kernelname the name of the kernel e.g. mandelbrot, julia_set or mandelbrot_alt
	 * @param sourceFilename the filename of the opencl file
	 */
	OCLRenderer(size_t width, size_t height, size_t gpuNum = 0, const std::string &kernelname = "mandelbrot",
	            const std::string &sourceFilename = "kernels/default.cl");

	/**
	 * opens and compiles a program with the given filename and the given kernel name
	 */
	bool openProgram(const std::string &filename, const std::string &kernelname);

	/**
	 * prints all OpenCL devices
	 */
	static void printAllDevices();

	/**
	 * renders to the texture
	 *
	 * @param refresh if set to true the texture gets flushed and starts with 1 samples, otherwise there will be generated continously new samples for AA
	 */
	void render(bool refresh);

	const Texture &getTexture() const;

	/**
	 * resizes the opencl buffers and the texture
	 *
	 * @param width the width of the desired texture size
	 * @param height the height of the desired texture size
	 */
	void reshape(size_t width, size_t height);

	double getZoom() const;

	void setZoom(double zoom);

	const cl_double2 &getPos() const;

	void setPos(double x, double y);

	const cl_float3 &getColor() const;

	void setColor(const cl_float3 &color);

	int getSampleCount() const;

	cl_int getIterations() const;

	void setIterations(cl_int iterations);

	std::shared_ptr<std::vector<cl_float>> getImage() const ;
};
