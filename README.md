# interference
Cross-platform C++ library - universal neurobiology-based machine learning framework.

Version 1.1.0

----------------------------------------------------------------
### PLATFORMS
- Windows (MinGW) (x86)
- Linux (x86, ARM)

----------------------------------------------------------------
### REQUIREMENTS
- CMake 3.12 or newer
- g++ 7.4.0 or newer (MinGW under Windows)
- [LodePNG library](https://github.com/lvandeve/lodepng) (for building inn_vision sample only)

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

----------------------------------------------------------------
### RELEASE NOTES
#### 1.1.0
- Interference library performance increased about 15 times
- Added multithreading support for multicore CPUs
- Added experimental support of ARM architecture
- Added ultimate example of image recognition system
- Exceptions raised by inn::Error class are more informative now
- Added doCheckSignal(), doResetSignalController(), getSignal() and getTime() methods for inn::NeuralNet::Link class
- Fixed doAddNeuron(...) method in inn::NeuralNet class (now allowed feedback links)
- Added doComparePatterns() and getNeuronCount() methods for inn::NeuralNet class
- Added new modes of neuron link definition: LinkDefinitionRange and LinkDefinitionRangeNext
- Now getEntries() and getReceptors() methods in inn::Neuron are named as getEntry(...) and getReceptor(...)
- Added doPrepare(), setk1(...) and setk2(...) methods for inn::Neuron, inn::Neuron::Entry and inn::Neuron::Synaps classes
- Added setNeurotransmitterType(...) method for inn::Neuron::Entry and inn::Neuron::Synaps classes
- Added getQSize() method for inn::Neuron::Synaps class
- Added setk3(...) and getdFi() methods for inn::Neuron::Receptor class
- Added getReceptorInfluenceValue(...) and getSynapticSensitivityValue(...) static methods for inn::Neuron::System class
- Added new arithmetic operations methods for inn::Position class
- Added doZeroPosition() and getDistanceFrom(...) methods for inn::Position class

Release notes for all releases can be found in RELEASES.md file.
