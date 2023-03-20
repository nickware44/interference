/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     16.03.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <iomanip>
#include "inn/system.h"
#include "inn/neuralnet.h"
#include "bmp.hpp"

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

std::vector<std::vector<double>> doBuildInputVector(std::vector<BMPImage> &images, std::vector<std::string> &texts) {
    std::vector<std::vector<double>> input;
    for (int d = 0,t = 0; d < images[0].size(); d+=2, t++) {
        input.emplace_back();
        for (int i = 0; i < images.size(); i++) {
            for (int s = 0; s < 2; s++) {
                auto rgbn = std::vector<double>({images[i][d+s][0]/255., images[i][d+s][1]/255., images[i][d+s][2]/255.});
                auto HSI = RGB2HSI(rgbn[0], rgbn[1], rgbn[2]);
                input.back().emplace_back(HSI[0]/(2*M_PI));
                input.back().emplace_back(HSI[1]);
                input.back().emplace_back(HSI[2]);
            }

            auto words = texts[i];
            if (t < words.size()) input.back().emplace_back(double(words[t])/255);
            else input.back().emplace_back(0);
        }
    }
    return input;
}

int main() {
    inn::System::setComputeBackend(inn::System::ComputeBackends::Multithread, 2);
    constexpr uint8_t COUNT = 3;
    constexpr uint16_t IMAGE_SIZE = 128*128;
    constexpr char STRUCTURE_PATH[128] = "../samples/multimodal/structure.json";
    constexpr char IMAGES_PATH[128] = "../samples/multimodal/images/";
    std::vector<std::string> names = {"mug with a melons", "mug with a big apple", "tomato"};
    std::vector<std::string> variants = {"mug with a melons", "mug wth a meln", "mug with a big apple", "mug w th a bi ap ple", "tomato", "tomto"};

    // load neural network structure from file
    std::ifstream structure(STRUCTURE_PATH);
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);

    // replicate neurons
    for (int i = 2; i <= COUNT; i++) NN -> doReplicateEnsemble("A1", "A"+std::to_string(i), true);

    std::cout << "Threads     : " << inn::System::getComputeBackendParameter()+1 << std::endl;
    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count: " << NN->getNeuronCount() << std::endl;
    std::cout << std::endl;

    std::cout << "List of objects:" << std::endl;
    std::vector<BMPImage> images;
    for (int b = 1; b <= COUNT; b++) {
        auto image = doReadBMP(IMAGES_PATH+std::to_string(b)+".bmp");
        images.push_back(image);
        std::cout << b << ". Image [" << b << ".bmp] - " << names[b-1] << std::endl;
        if (image.size() != IMAGE_SIZE) {
            std::cout << "Error loading image " << b << ".bmp" << std::endl;
            return 1;
        }
    }
    auto input = doBuildInputVector(images, names);

    auto T = getTimestampMS();

    // teach the neural network
    NN -> doLearn(input);

    // detect
    std::cout << std::endl << "Detecting objects:" << std::endl;
    for (int b = 1; b <= COUNT; b++) {
        std::string name = std::to_string(b)+".bmp";
        auto image = doReadBMP(IMAGES_PATH+name);
        std::vector<BMPImage> rimages;
        for (int i = 1; i <= COUNT; i++) rimages.push_back(image);

        std::cout << "Image [" << b << ".bmp]" << std::endl;

        for (int e = 0; e < variants.size(); e++) {
            std::cout << std::setw(35) << std::left << std::to_string(e+1)+". "+variants[e];

            std::vector<std::string> rvariants;
            for (int i = 1; i <= COUNT; i++) rvariants.push_back(variants[e]);

            auto rinput = doBuildInputVector(rimages, rvariants);

            NN -> doRecognise(rinput);

            auto patterns = NN -> doComparePatterns();
            auto r = std::max_element(patterns.begin(), patterns.end());
            if (std::distance(patterns.begin(), r) == b-1) {
                std::cout << "[CORRECT ANSWER]" << std::endl;
            } else {
                std::cout << "[INCORRECT ANSWER]" << std::endl;
                // Uncomment to print the response of output neurons to the input data (response - values [0, 1], 0 - minimum response, 1 - maximum response)
//                 std::cout << "Difference for outputs:" << std::endl;
//                 for (int i = 0; i < patterns.size(); i++) std::cout << (i+1) << ". " << patterns[i] << std::endl;
            }
        }
        std::cout << std::endl;
    }

    T = getTimestampMS() - T;
    std::cout << "Done in " << T << " ms" << std::endl;
    return 0;
}
