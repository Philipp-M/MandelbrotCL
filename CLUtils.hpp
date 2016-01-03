#include <CL/cl.hpp>
#include <cmath>

namespace cl
{
    extern std::string memoryTypeString(cl_device_local_mem_type memType);

    extern std::string deviceTypeString(cl_device_type deviceType);

    extern std::string errorString(cl_int error);

	inline unsigned int nextPowOfTwo(unsigned int n)
	{
		return 1 << ((unsigned int) ceil(log2(n)));
	}

	inline unsigned int nextDivisible(unsigned long size, unsigned long localSize)
	{
		return (int) ceil((double) size / localSize) * localSize;
	}
}
