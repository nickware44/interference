
#include <fstream>
#include <inn/neuralnet.h>
#include <unistd.h>

#define NN_OUTPUT_OK 231.363

int main() {
    inn::setComputeBackend(inn::ComputeBackends::Multithread, 1);

    std::ifstream structure("../../test/structure2.json");
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);
    std::cout << "Model name: " << NN->getName() << std::endl;
    std::cout << "Model desc: " << NN->getDescription() << std::endl;
    std::cout << "Model ver : " << NN->getVersion() << std::endl;

    std::cout << "Running first step" << std::endl;
    NN -> doSignalSend({500, 500});
    std::cout << "Running second step" << std::endl;
    NN -> doSignalSend({200, 200});
    sleep(100);
    auto Y = NN -> doSignalReceive();
    std::cout << "Neural net output: " << Y[0] << std::endl;
    if (fabs(Y[0]-NN_OUTPUT_OK) < 1e-4) {
        std::cout << "Test [PASSED]" << std::endl;
    } else {
        std::cout << "Test [FAILED]" << std::endl;
        std::cout << "Neural net output is not " << NN_OUTPUT_OK << std::endl;
    }

    delete NN;
    return 0;
}
