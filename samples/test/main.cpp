
#include <fstream>
#include <inn/neuralnet.h>

#define NN_OUTPUT_OK 251.145
#define NN_TEST_COUNT 2

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
                                                                                time_since_epoch()).count();
}

int main() {
    int TestDoneCount = 0, TestTotalCount = NN_TEST_COUNT;
    bool PassedFlag;
    inn::setVerbosityLevel(1);
    inn::setComputeBackend(inn::ComputeBackends::Default);

    std::ifstream structure("../samples/test/structure.json");
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);

    for (int i = 2; i < 101; i++) {
        NN -> doReplicateEnsemble("A1", "A"+std::to_string(i));
    }

    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count: " << NN->getNeuronCount() << std::endl;
    std::cout << std::endl;

    // creating data array
    std::vector<std::vector<double>> X;
    for (int i = 0; i < 50; i++) {
        X.push_back({1.02094, 1.02094});
    }

    std::cout << "Superstructure test: ";
    auto T = getTimestampMS();
    auto Y = NN -> doLearn(X);
    T = getTimestampMS() - T;
    std::cout << "done [" << T << " ms]" << std::endl;

    PassedFlag = true;
    for (auto &y: Y) {
        if (fabs(y-NN_OUTPUT_OK) > 1e-3) {
            std::cout << "Output value " << y << " is not " << NN_OUTPUT_OK << std::endl;
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
    inn::setComputeBackend(inn::ComputeBackends::Multithread, 6);
    T = getTimestampMS();
    Y = NN ->doLearn(X);
    T = getTimestampMS() - T;
    std::cout << "done [" << T << " ms]" << std::endl;

    PassedFlag = true;
    for (auto &y: Y) {
        if (fabs(y-NN_OUTPUT_OK) > 1e-4) {
            std::cout << "Output value " << y << " is not " << NN_OUTPUT_OK << std::endl;
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
    std::cout << "Tests passed: [" << TestDoneCount << "/" << TestTotalCount << "]" << std::endl;
    delete NN;
    return 0;
}
