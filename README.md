# Mandelbrot CL #

A realtime mandelbrot renderer, written in C++11 with SDL2, OpenGL and OpenCL.

![Mandelbrot](https://raw.githubusercontent.com/Philipp-M/MandelbrotCL/master/images/mandelbrot.png)

## Features ##
double precision support, just uncomment the following in CMakeLists.txt

```cmake
#add_definitions( -DUSE_DOUBLE )
```

The renderer renders the Mandelbrot Set with continuously new samples for nice Antialiasing.
A tent filter with a combined Tausworthe and Linear Congruential Generator random generator was used for achieving this.
This kind of 'overkill' feature is build in since the main goal is a realtime Pathtracer.

There is also an alternative Mandelbrot implementation and a Julia Set in the opencl file
just adjust this line to either 'julia_set' or 'mandelbrot_alt' in GLMain.cpp
```cpp
oclRenderer.reset(new OCLRenderer(width, height, 0, "mandelbrot", "kernels/default.cl"));
```

## Controls ##

* Mouse
    * **Left button + motion** drag the screen
    * **Right button + vertical motion** zoom in and out
    * **Mouse wheel** zoom
* Keyboard
    * **p** save rendered image
    * **c** new random colors
    * **+** increase the iterations by a factor of 1.25 (default 300)
    * **-** decrease the iterations by a factor of 0.8
