[![CMake](https://github.com/Glebanister/float-matrix/actions/workflows/cmake.yml/badge.svg)](https://github.com/Glebanister/float-matrix/actions/workflows/cmake.yml)

# Float matrix addition

Application for JetBrains Research autumn project. The addition of sparse float matrices
on GPU.

## Installation

First, make sure `git`, `cmake 15` and `g++` are working fine on your machine.
Also, `Boost 1.76` is required for build and `OpenCL` package,
they can be installed by typing

```bash
sudo apt update
sudo apt install libboost-all-dev
sudo apt install ocl-icd-opencl-dev
```

Then type following commands in your terminal

```bash
git clone --recursive https://github.com/Glebanister/float-matrix
cd float-matrix
mkdir build && cd build
cmake ..
```

## Testing

For the tests to be successful it is necessary that
OpenCL devices are available on the device,
otherwise error `No OpenCL device found` or `OpenCL device not avaiable` may occur.
I personally executed tests on `Apple M1` and everything worked out
surprisingly well.

All float values are compared with epsilon `floatMatrix::utility::FLOAT_EPS`
which is `1e-5`.
If you set the accuracy higher, the tests whose values are entered manually will not converge.

```bash
./build/test-float-matrix
```

## Implementation

CooMatrix is a class (represented in [src/CooMatrix.hpp](./src/CooMatrix.hpp)) that represents a sparse matrix in [COO format](https://en.wikipedia.org/wiki/Sparse_matrix#Coordinate_list_(COO)).
It is **not** assumed that the cells are somehow sorted in the beginning.

First, the matrix must be filled with values, for this purpose an array of cells can be passed to its constructor,
or it can be filled with values manually.

```c++
using FloatMatrix = floatMatirx::CooMatrix;
using DeviceCells = FloatMatrix::DeviceCells

FloatMatrix mtx(DeviceCells{
    {0, 0, 1.0},
    {2, 2, 0.0},
    {0, 1, 1.1},
});

mtx.set(1, 1, 2.5);
mtx.set(2, 1, 2.5);

assert(mtx.get(0, 0) == 1.0); // It is float values, so be careful with comparison

// The resulting matrix is:
//
// 1.0 1.1 0.0
// 0.0 2.5 0.0
// 0.0 2.5 0.0
```

Also, the matrix can be read from the input stream or can be written there.

```c++
FloatMatrix mtx(DeviceCells{{0, 0, 1.0}, {0, 1, 2.3}});

std::stringstream ss;
ss << mtx;
FloatMatrix deserialized;
ss >> deserialized;

// ss.str() is probably:
// 2
// 0 0 1.0 0 1 2.3
```

The addition can be done in simple way (without OpenCL)

```c++
A.add(B);
```

Or the hard way. In this case, `boost::compute::command_queue` is required.
Below you can find simple way to initialize it.

```c++
boost::compute::device device = boost::compute::system::default_device();
boost::compute::context context(device);
boost::compute::command_queue queue(context, device);

A.add(B, queue);
```
