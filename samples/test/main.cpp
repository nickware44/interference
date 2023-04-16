/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     30.01.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <inn/neuralnet.h>

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
                                                                                time_since_epoch()).count();
}

int main() {
    constexpr int TOTAL_TEST_COUNT = 3;
    constexpr float REFERENCE_OUTPUT = 10.6149;

    int TestDoneCount = 0;
    bool PassedFlag;
    inn::System::setVerbosityLevel(1);
    inn::System::setComputeBackend(inn::System::ComputeBackends::Default);

    std::ifstream structure("../samples/test/structure.json");
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);

    for (int i = 2; i < 101; i++) {
        NN -> doReplicateEnsemble("A1", "A"+std::to_string(i));
    }
    NN -> doStructurePrepare();

    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count: " << NN->getNeuronCount() << std::endl;
    std::cout << std::endl;

    // creating data array
    std::vector<std::vector<float>> X;
    for (int i = 0; i < 170; i++) {
        X.push_back({50, 50});
    }

    std::cout << "Superstructure test: ";
    auto T = getTimestampMS();
    auto Y = NN -> doLearn(X);
    T = getTimestampMS() - T;
    std::cout << "done [" << T << " ms]" << std::endl;

    PassedFlag = true;
    for (auto &y: Y) {
        if (fabs(y-REFERENCE_OUTPUT) > 1e-4) {
            std::cout << "Output value " << y << " is not " << REFERENCE_OUTPUT << std::endl;
            std::cout << "[FAILED]" << std::endl;
            PassedFlag = false;
            break;
        }
    }
    if (PassedFlag) {
        TestDoneCount++;
        std::cout << "[PASSED]" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Superstructure multithread test: ";
    inn::System::setComputeBackend(inn::System::ComputeBackends::Multithread, 2);
    T = getTimestampMS();
    Y = NN ->doLearn(X);
    T = getTimestampMS() - T;
    std::cout << "done [" << T << " ms]" << std::endl;

    PassedFlag = true;
    for (auto &y: Y) {
        if (std::fabs(y-REFERENCE_OUTPUT) > 1e-4) {
            std::cout << "Output value " << y << " is not " << REFERENCE_OUTPUT << std::endl;
            std::cout << "[FAILED]" << std::endl;
            PassedFlag = false;
            break;
        }
    }
    if (PassedFlag) {
        TestDoneCount++;
        std::cout << "[PASSED]" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Superstructure OpenCL test: ";
    inn::System::setComputeBackend(inn::System::ComputeBackends::OpenCL);
    T = getTimestampMS();
    Y = NN ->doLearn(X);
    T = getTimestampMS() - T;
    std::cout << "done [" << T << " ms]" << std::endl;

    PassedFlag = true;
    for (auto &y: Y) {
        if (fabs(y-REFERENCE_OUTPUT) > 1e-3) {
            std::cout << "Output value " << y << " is not " << REFERENCE_OUTPUT << std::endl;
            std::cout << "[FAILED]" << std::endl;
            PassedFlag = false;
            break;
        }
    }
    if (PassedFlag) {
        TestDoneCount++;
        std::cout << "[PASSED]" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Tests passed: [" << TestDoneCount << "/" << TOTAL_TEST_COUNT << "]" << std::endl;
    delete NN;

    if (!PassedFlag) return 1;
    return 0;
}
