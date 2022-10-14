
#include <fstream>
#include <inn/neuralnet.h>

int main() {
    std::ifstream structure("../../test/structure.json");
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);
    std::cout << "Model name: " << NN->getName() << std::endl;
    std::cout << "Model desc: " << NN->getDescription() << std::endl;
    std::cout << "Model ver : " << NN->getVersion() << std::endl;

    NN -> doSignalSend({1, 1});

    delete NN;
    return 0;
}
