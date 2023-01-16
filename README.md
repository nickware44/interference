<p align="center">
  <img width="750" height="200" src="https://nickware.group/repository/products/inn/logo.png"><br><br>
Cross-platform C++ library - universal neurobiology-based machine learning framework.<br>
Version 2.0.0
</p>


----------------------------------------------------------------
### PLATFORMS
|             | x86 | armle.v7 |   |
|:-----------:|:---:|:--------:|:---:|
| **Windows** |  +  |          |   |
|  **Linux**  |  +  |    +     |   |
|   **QNX**   |  +  |    +     |   |

----------------------------------------------------------------
### COMPUTING
- Native CPU (single thread)
- Native CPU (multithread)

----------------------------------------------------------------
### REQUIREMENTS
- CMake 3.12 or newer
- g++ 7.4.0 or newer (MinGW under Windows)

----------------------------------------------------------------
### LICENCE
Interference library is distributed under the MIT Licence.

inn_vision example uses the part of COIL-100 dataset.

"Columbia Object Image Library (COIL-100)," S. A. Nene, S. K. Nayar and H. Murase, Technical Report CUCS-006-96, February 1996.
http://www1.cs.columbia.edu/CAVE/software/softlib/coil-100.php

----------------------------------------------------------------
### HOW TO BUILD
#### Building library for Windows (MinGW)
Run CMD and follow this steps:
1. Prepare build directory (%INTERFERENCE_ROOT% is root directory of Interference library files, %BUILD_TYPE% is "debug" or "release")
```
> cd %INTERFERENCE_ROOT%
> mkdir cmake-build-%BUILD_TYPE%
> cd cmake-build-%BUILD_TYPE%
```
2. Configure build files
```
> cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_MAKE_PROGRAM=make.exe -G "MinGW Makefiles" ..
```
3. Build the library
```   
> make
```
4. Install the library in comfortable location
```
> make DESTDIR=.. install
```
#### Building library for Linux
Run terminal and follow this steps:
1. Prepare build directory (%INTERFERENCE_ROOT% is root directory of Interference library files, %BUILD_TYPE% is "debug" or "release")
```
> cd %INTERFERENCE_ROOT%
> mkdir cmake-build-%BUILD_TYPE%
> cd cmake-build-%BUILD_TYPE%
```
2. Configure build files
```
> cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
```
3. Build the library
```
> make
```
4. Install the library in a global location
```
> sudo make install
```
#### Building an example
To compile an example you need to run this commands:
```
> cd %INTERFERENCE_ROOT%/samples/%SAMPLE_NAME%
> mkdir cmake-build-%BUILD_TYPE%
> cd cmake-build-%BUILD_TYPE%
```
Repeat steps 2 and 3 from library building instruction and run ```make DESTDIR=.. install``` (for both platforms).
Now you can run example using this commands:
```
> cd ../bin/
> ./%SAMPLE_NAME%_sample
```


