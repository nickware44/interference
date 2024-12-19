# Getting started

<p align="center">
  <img src="https://nickware.group/repository/products/indk/logo.png"><br><br>
</p>


## Platforms
|                                | x86 | aarch64 | armle.v7 | PowerPC | e2k | MIPS |
|:------------------------------:|:---:|:-------:|:--------:|:-------:|:---:|:----:|
|          **Windows**           |  +  |         |          |         |     |      |
|           **Linux**            |  +  |    +    |    +     |         |  +  |      |
|            **QNX**             |  +  |         |    +     |         |     |      |
| **Secured RTOS KPDA 10964-01** |  +  |    +    |    +     |    +    |  +  |  +   |


## Computing
- Native CPU (single thread)
- Native CPU (multithread)
- OpenCL


## Requirements
- CMake 3.12 or newer
- g++ 8.3.0 or newer (MinGW under Windows)

----------------------------------------------------------------
## Building library and samples
Note that %INTERFERENCE_ROOT% is root directory of Interference library files, %BUILD_TYPE% is "debug" or "release". After the last step, library and binaries will be in %INTERFERENCE_ROOT%/lib and %INTERFERENCE_ROOT%/bin respectively.
### Building for Windows (MinGW)
Run CMD and follow this steps:
1. Prepare build directory
```
cd %INTERFERENCE_ROOT%
mkdir cmake-build-%BUILD_TYPE%
cd cmake-build-%BUILD_TYPE%
```
2. Configure build files
```
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_MAKE_PROGRAM=make.exe -G "MinGW Makefiles" ..
```
3. Build and install the library
```
make install
```
### Building for Linux
Run terminal and follow this steps:
1. Prepare build directory
```
cd %INTERFERENCE_ROOT%
mkdir cmake-build-%BUILD_TYPE%
cd cmake-build-%BUILD_TYPE%
```
2. Configure build files
```
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
```
3. Build and install the library
```
make install
```
