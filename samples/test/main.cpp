/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     30.01.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <fstream>
#include <indk/neuralnet.h>
#include <indk/profiler.h>
#include <iomanip>


indk::NeuralNet *NN;
std::vector<std::vector<float>> X;

std::vector<std::tuple<indk::System::ComputeBackends, int, std::string>> backends = {
        std::make_tuple(indk::System::ComputeBackends::Default, 0, "singlethread"),
        std::make_tuple(indk::System::ComputeBackends::Multithread, 2, "multithread"),
        std::make_tuple(indk::System::ComputeBackends::OpenCL, 0, "OpenCL"),
};

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
                                                                                time_since_epoch()).count();
}

void doLoadModel(const std::string& path, int size) {
    std::ifstream structure(path);
    NN -> setStructure(structure);

    for (int i = 2; i < size; i++) {
        NN -> doReplicateEnsemble("A1", "A"+std::to_string(i));
    }
    NN -> doStructurePrepare();

    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count: " << NN->getNeuronCount() << std::endl;
    std::cout << std::endl;
}

int doTest(float ref) {
    auto T = getTimestampMS();
    auto Y = NN -> doLearn(X);
    T = getTimestampMS() - T;
    std::cout << std::setw(20) << std::left << "done ["+std::to_string(T)+" ms] ";

    bool passed = true;
    for (auto &y: Y) {
        if (std::fabs(y.first-ref) > 1e-3) {
            std::cout << "[FAILED]" << std::endl;
            std::cout << "Output value " << y.first << " is not " << ref << std::endl;
            std::cout << std::endl;
            passed = false;
            break;
        }
    }
    if (passed) {
        std::cout << "[PASSED]" << std::endl;
    }

    return passed;
}

int doTests(const std::string& name, float ref) {
    int count = 0;

    for (auto &b: backends) {
        NN -> doReset();
        std::cout << std::setw(50) << std::left << name+" ("+std::get<2>(b)+"): ";
        indk::System::setComputeBackend(std::get<0>(b), std::get<1>(b));
        count += doTest(ref);
    }
    std::cout << std::endl;

    return count;
}

int main() {
    constexpr unsigned STRUCTURE_COUNT                      = 2;
    constexpr float SUPERSTRUCTURE_TEST_REFERENCE_OUTPUT    = 0.0291;
    constexpr float BENCHMARK_TEST_REFERENCE_OUTPUT         = 2.7622;
    const unsigned TOTAL_TEST_COUNT                         = STRUCTURE_COUNT*backends.size();

    int count = 0;
    indk::System::setVerbosityLevel(1);
    NN = new indk::NeuralNet();

    // creating data array
    for (int i = 0; i < 170; i++) {
        X.push_back({50, 50});
    }

    // running tests
    std::cout << "=== SUPERSTRUCTURE TEST ===" << std::endl;
    doLoadModel("structures/structure_general.json", 101);
    count += doTests("Superstructure test", SUPERSTRUCTURE_TEST_REFERENCE_OUTPUT);

    std::cout << "=== BENCHMARK ===" << std::endl;
    doLoadModel("structures/structure_bench.json", 10001);
    count += doTests("Benchmark", BENCHMARK_TEST_REFERENCE_OUTPUT);

    std::cout << std::endl;
    std::cout << "Tests passed: [" << count << "/" << TOTAL_TEST_COUNT << "]" << std::endl;
    delete NN;

    if (count != TOTAL_TEST_COUNT) return 1;
    return 0;
}
