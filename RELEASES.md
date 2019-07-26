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

#### 1.0.8
- Fixed inn::Position operators definition
- Fixed neurons priority assignment algorithm in inn::NeuralNet
- Added getXm() method for inn::Neuron
- Added destructors for inn::NeuralNet, inn::Neuron and inn::Neuron::Entry
- Added doFinalize() method for inn::NeuralNet
- Improved data finalization process
- Type fixes in inn::NeuralNet
- inn::Neuron synapse and receptor position definition fixes (doCreateNewSynaps() and doCreateNewReceptor() methods)
- Added new inn::Position constructor

#### 1.0.3
- Fixed file headers
- Added implementation of inn::Error (exception system)
- Updated library structure
