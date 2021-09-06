[![CMake](https://github.com/Glebanister/float-matrix/actions/workflows/cmake.yml/badge.svg)](https://github.com/Glebanister/float-matrix/actions/workflows/cmake.yml)

# Float matrix addition

Application for JetBrains Research autumn project.

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
