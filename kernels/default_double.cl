//------------------------------------------------------------------------------
// Random number generator
// combined Tausworthe and LCG generator
//------------------------------------------------------------------------------

inline uint tausStep(uint* z, int S1, int S2, int S3, uint M)
{
	uint b=((((*z) << S1) ^ (*z)) >> S2);
	return *z = ((((*z) & M) << S3) ^ b);
}

inline uint lcgStep(uint* z, uint A, uint C)
{
	return *z=(A*(*z)+C);
}
inline float rand(uint4* z)
{
	// Combined period is lcm(p1,p2,p3,p4)~ 2^121
	return 2.3283064365387e-10f * (float)(              // Periods
			tausStep((uint*)((uint*)z+0), 13, 19, 12, 4294967294) ^  // p1=2^31-1
			tausStep((uint*)((uint*)z+1), 2, 25, 4, 4294967288) ^    // p2=2^30-1
			tausStep((uint*)((uint*)z+2), 3, 11, 17, 4294967280) ^   // p3=2^28-1
			lcgStep((uint*)((uint*)z+3), 1664525, 1013904223)        // p4=2^32
	);
}

inline double2 complexMul(double2 a, double2 b)
{
	return (double2)(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

inline float4 getColor(float3 col, int i, float absVal)
{
	// The color scheme here is based on one
	// from Inigo Quilez's Shader Toy:
	float co = (float)i + 1.0 - log2(.5f * log2(absVal));
	co = sqrt(co / 256.0f);
	return (float4)(.5 + .5 * (cos(6.2831f * co + col.x) )+ 0.2f*sin(0.1f*6.2831f * co*co*25.0f + col.x),
	                .5 + .5 * (cos(6.2831f * co + col.y) )+ 0.2f*sin(0.1f*6.2831f * co*co*25.0f + col.y),
	                .5 + .5 * (cos(6.2831f * co + col.z) )+ 0.2f*sin(0.1f*6.2831f * co*co*25.0f + col.z),
	                1.0);
}

kernel void mandelbrot(__read_write image2d_t image, global float4* imageRaw, global uint4* randStates, const float3 color, const int width, const int height, const int iterations,
                       const double zoom, const double2 pos, int sampleCount)
{
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	if (x < width && y < height)
	{
		const uint imgIndex = y*width + x;
		uint4 r = randStates[imgIndex];
		const int2 coords = (int2)(x, y);
		const double r1 = 2.0*rand(&r), dx = r1<1.0 ? sqrt(r1)-1.0: 1.0-sqrt(2.0-r1);
		const double r2 = 2.0*rand(&r), dy = r2<1.0 ? sqrt(r2)-1.0: 1.0-sqrt(2.0-r2);
		const double xN = zoom * ((x + 0.5 + dx/2.0) / width + pos.x);
		const double yN = zoom * ((y + 0.5 + dy/2.0) / width + pos.y);
		const double maxAbsolute = 200.0;
		double xNtmp = xN;
		double yNtmp = yN;
		double xxN = xN * xN;
		double yyN = yN * yN;
		double xyN = xN * yN;
		double absolute = xxN + yyN;
		int i;
		for (i = 0; i < iterations && absolute <= maxAbsolute; ++i)
		{
			xNtmp = xxN - yyN + xN;
			yNtmp = xyN + xyN + yN;
			xxN = xNtmp * xNtmp;
			yyN = yNtmp * yNtmp;
			xyN = xNtmp * yNtmp;
			absolute = xxN + yyN;
		}
		float4 val;
		if(i == iterations)
			val = (sampleCount - 1 ? imageRaw[imgIndex] : (float4)(0.0f, 0.0f, 0.0f, 0.0f)) + (float4)(0.0f,0.0f,0.0f,1.0f);
		else
			val = (sampleCount - 1 ? imageRaw[imgIndex] : (float4)(0.0f, 0.0f, 0.0f, 0.0f)) + (float4)getColor(color,i,(float)absolute);
		imageRaw[imgIndex] = val;
		randStates[imgIndex] = r;

		write_imagef(image, coords, val/(float)sampleCount);
	}
}

kernel void julia_set(__read_write image2d_t image, global float4* imageRaw, global uint4* randStates, const float3 color, const int width, const int height, const int iterations,
                       const double zoom, const double2 pos, int sampleCount)
{
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	if (x < width && y < height)
	{
		const uint imgIndex = y*width + x;
		uint4 r = randStates[imgIndex];
		const int2 coords = (int2)(x, y);
		const double r1 = 2.0*rand(&r), dx = r1<1.0 ? sqrt(r1)-1.0: 1.0-sqrt(2.0-r1);
		const double r2 = 2.0*rand(&r), dy = r2<1.0 ? sqrt(r2)-1.0: 1.0-sqrt(2.0-r2);
		const double xN = zoom * ((x + 0.5 + dx/2.0) / width + pos.x);
		const double yN = zoom * ((y + 0.5 + dy/2.0) / width + pos.y);
		const double maxAbsolute = 200.0;
		const double cr = -0.53060;
		const double ci = -0.50340;
		double xNtmp = xN;
		double yNtmp = yN;
		double xxN = xN * xN;
		double yyN = yN * yN;
		double xyN = xN * yN;
		double absolute = xxN + yyN;
		int i;
		for (i = 0; i < iterations && absolute <= maxAbsolute; ++i)
		{
			xNtmp = xxN - yyN + cr;
			yNtmp = xyN + xyN + ci;
			xxN = xNtmp * xNtmp;
			yyN = yNtmp * yNtmp;
			xyN = xNtmp * yNtmp;
			absolute = sqrt(xxN + yyN);
		}
		float4 val;
		if(i == iterations)
			val = (sampleCount - 1 ? imageRaw[imgIndex] : (float4)(0.0f, 0.0f, 0.0f, 0.0f)) + (float4)(0.0f,0.0f,0.0f,1.0f);
		else
			val = (sampleCount - 1 ? imageRaw[imgIndex] : (float4)(0.0f, 0.0f, 0.0f, 0.0f)) + (float4)getColor(color,i,(float)absolute);
		imageRaw[imgIndex] = val;
		randStates[imgIndex] = r;

		write_imagef(image, coords, val/(float)sampleCount);
	}
}

kernel void mandelbrot_alt(__read_write image2d_t image, global float4* imageRaw, global uint4* randStates, const float3 color, const int width, const int height, const int iterations,
						   const double zoom, const double2 pos, int sampleCount)
{
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	if (x < width && y < height)
	{
		int i;

		const uint imgIndex = y*width + x;
		uint4 r = randStates[imgIndex];
		const int2 coords = (int2)(x, y);
		const double r1 = 2.0*rand(&r), dx = r1<1.0 ? sqrt(r1)-1.0: 1.0-sqrt(2.0-r1);
		const double r2 = 2.0*rand(&r), dy = r2<1.0 ? sqrt(r2)-1.0: 1.0-sqrt(2.0-r2);
		double2 z = (double2)(0.0f, 0.0f);
		const double2 c = (double2)((double) zoom * ((double) (x + 0.5 + dx/2.0) / width + pos.x),
		                            (double) zoom * ((double) (y + 0.5 + dy/2.0) / width + pos.y));
		const double maxAbsolute = 200.0f;
		double absolute;
		for (i = 0; i < iterations && (absolute = dot(z, z)) <= maxAbsolute; i++)
		{
			z = complexMul(z, z) + c;
		}
		float4 val;
		if(i == iterations)
			val = (sampleCount - 1 ? imageRaw[imgIndex] : (float4)(0.0f, 0.0f, 0.0f, 0.0f)) + (float4)(0.0f,0.0f,0.0f,1.0f);
		else
			val = (sampleCount - 1 ? imageRaw[imgIndex] : (float4)(0.0f, 0.0f, 0.0f, 0.0f)) + (float4)getColor(color,i,(float)absolute);
		imageRaw[imgIndex] = val;
		randStates[imgIndex] = r;

		write_imagef(image, coords, val/(float)sampleCount);
	}
}

