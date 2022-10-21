
#include <fstream>
#include <inn/neuralnet.h>
#include <unistd.h>

#define NN_OUTPUT_OK 164.322 //0.0216481 //1.32076

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
                                                                                time_since_epoch()).count();
}

int main() {
    inn::setComputeBackend(inn::ComputeBackends::Default, 4);

    std::ifstream structure("../../test/structure.json");
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);
    std::cout << "Model name: " << NN->getName() << std::endl;
    std::cout << "Model desc: " << NN->getDescription() << std::endl;
    std::cout << "Model ver : " << NN->getVersion() << std::endl;
    //sleep(1);
    auto T = getTimestampMS();
    for (int i = 0; i < 20; i++) {
        std::cout << "Running step " << i << std::endl;
        NN -> doSignalSend({1, 1});
    }
    T = getTimestampMS() - T;
    std::cout << "Time elapsed: " << T << " ms" << std::endl;
    sleep(5);
    auto Y = NN -> doSignalReceive();
    std::cout << "Neural net output: " << Y[0] << std::endl;
    if (fabs(Y[0]-NN_OUTPUT_OK) < 1e-3) {
        std::cout << "Test [PASSED]" << std::endl;
    } else {
        std::cout << "Test [FAILED]" << std::endl;
        std::cout << "Neural net output is not " << NN_OUTPUT_OK << std::endl;
    }

    delete NN;
    return 0;
}
