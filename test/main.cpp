
#include <fstream>
#include <inn/neuralnet.h>
#include <unistd.h>

#define NN_OUTPUT_OK 2.66677 //0.0216481 //1.32076

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
                                                                                time_since_epoch()).count();
}

int main() {
    inn::setComputeBackend(inn::ComputeBackends::Multithread, 6);

    std::ifstream structure("../../test/structure2.json");
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);
    NN -> doReplicateEnsemble("A1", "A2");
    NN -> doReplicateEnsemble("A1", "A3");
    NN -> doReplicateEnsemble("A1", "A4");
    NN -> doReplicateEnsemble("A1", "A5");
    NN -> doReplicateEnsemble("A1", "A6");
    NN -> doReplicateEnsemble("A1", "A7");
    NN -> doReplicateEnsemble("A1", "A8");
    NN -> doReplicateEnsemble("A1", "A9");
    NN -> doReplicateEnsemble("A1", "A10");
    NN -> doReplicateEnsemble("A1", "A11");
    NN -> doReplicateEnsemble("A1", "A12");
    NN -> doReplicateEnsemble("A1", "A13");
    NN -> doReplicateEnsemble("A1", "A14");
    NN -> doReplicateEnsemble("A1", "A15");
    NN -> doReplicateEnsemble("A1", "A16");
    NN -> doReplicateEnsemble("A1", "A17");
    NN -> doReplicateEnsemble("A1", "A18");
    NN -> doReplicateEnsemble("A1", "A19");
    NN -> doReplicateEnsemble("A1", "A20");
    NN -> doReplicateEnsemble("A1", "A21");
    NN -> doReplicateEnsemble("A1", "A22");
    NN -> doReplicateEnsemble("A1", "A23");
    NN -> doReplicateEnsemble("A1", "A24");
    NN -> doReplicateEnsemble("A1", "A25");
    NN -> doReplicateEnsemble("A1", "A26");
    NN -> doReplicateEnsemble("A1", "A27");
    NN -> doReplicateEnsemble("A1", "A28");
    NN -> doReplicateEnsemble("A1", "A29");
    NN -> doReplicateEnsemble("A1", "A30");

    std::ifstream structure2("../../test/structure2.json");
    auto NN2 = new inn::NeuralNet();
    NN2 -> setStructure(structure2);
    NN2 -> doReplicateEnsemble("A1", "A2");
    NN2 -> doReplicateEnsemble("A1", "A3");
    NN2 -> doReplicateEnsemble("A1", "A4");
    NN2 -> doReplicateEnsemble("A1", "A5");
    NN2 -> doReplicateEnsemble("A1", "A6");
    NN2 -> doReplicateEnsemble("A1", "A7");
    NN2 -> doReplicateEnsemble("A1", "A8");
    NN2 -> doReplicateEnsemble("A1", "A9");
    NN2 -> doReplicateEnsemble("A1", "A10");
    NN2 -> doReplicateEnsemble("A1", "A11");
    NN2 -> doReplicateEnsemble("A1", "A12");
    NN2 -> doReplicateEnsemble("A1", "A13");
    NN2 -> doReplicateEnsemble("A1", "A14");
    NN2 -> doReplicateEnsemble("A1", "A15");
    NN2 -> doReplicateEnsemble("A1", "A16");
    NN2 -> doReplicateEnsemble("A1", "A17");
    NN2 -> doReplicateEnsemble("A1", "A18");
    NN2 -> doReplicateEnsemble("A1", "A19");
    NN2 -> doReplicateEnsemble("A1", "A20");
    NN2 -> doReplicateEnsemble("A1", "A21");
    NN2 -> doReplicateEnsemble("A1", "A22");
    NN2 -> doReplicateEnsemble("A1", "A23");
    NN2 -> doReplicateEnsemble("A1", "A24");
    NN2 -> doReplicateEnsemble("A1", "A25");
    NN2 -> doReplicateEnsemble("A1", "A26");
    NN2 -> doReplicateEnsemble("A1", "A27");
    NN2 -> doReplicateEnsemble("A1", "A28");
    NN2 -> doReplicateEnsemble("A1", "A29");
    NN2 -> doReplicateEnsemble("A1", "A30");

    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count: " << NN->getNeuronCount() << std::endl;

    std::vector<std::vector<double>> X;
    for (int i = 0; i < 2500; i++) {
        X.push_back({1, 1});
    }

    auto T = getTimestampMS();
    NN2 -> doSignalTransferAsync(X, [T](const std::vector<double>& Y) {
        std::cout << "Async neural net output: " << std::endl;
        for (auto &y: Y) std::cout << y << std::endl;
        auto Tx = getTimestampMS() - T;
        std::cout << "Time elapsed for async: " << Tx << " ms" << std::endl;
    });
    auto Y = NN -> doSignalTransfer(X);
    T = getTimestampMS() - T;
    std::cout << "Time elapsed: " << T << " ms" << std::endl;

    std::cout << "Neural net output: " << std::endl;
    for (auto &y: Y) std::cout << y << std::endl;
    if (fabs(Y[0]-NN_OUTPUT_OK) < 1e-3) {
        std::cout << "Test [PASSED]" << std::endl;
    } else {
        std::cout << "Test [FAILED]" << std::endl;
        std::cout << "Neural net output is not " << NN_OUTPUT_OK << std::endl;
    }

    delete NN;
    return 0;
}
