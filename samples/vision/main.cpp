/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 30.01.23
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <chrono>
#include "inn/system.h"
#include "inn/neuralnet.h"
#include "bmp.hpp"

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

std::vector<std::vector<double>> doBuildInputVector(std::vector<BMPImage> &images) {
    std::vector<std::vector<double>> input;
    for (int d = 0; d < images[0].size(); d+=8) {
        input.emplace_back();
        for (int i = 0; i < images.size(); i++) {
            for (int s = 0; s < 8; s++) {
                input.back().emplace_back(images[i][d+s][0]/255.);
                input.back().emplace_back(images[i][d+s][1]/255.);
                input.back().emplace_back(images[i][d+s][2]/255.);
            }
        }
    }
    return input;
}

int main() {
    inn::System::setVerbosityLevel(1);
    //inn::System::setComputeBackend(inn::System::ComputeBackends::Multithread, 6);

    constexpr uint8_t LEARNING_COUNT = 10;
    constexpr uint8_t TEST_COUNT = 10;
    constexpr char STRUCTURE_PATH[128] = "../samples/vision/structure.json";
    constexpr char IMAGES_LEARNING_PATH[128] = "../samples/vision/images/learn/";
    constexpr char IMAGES_TESTING_PATH[128] = "../samples/vision/images/test/";

    // load neural network structure from file
    std::ifstream structure(STRUCTURE_PATH);
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);

    // replicate neurons for classification
    for (int i = 2; i <= LEARNING_COUNT; i++) NN -> doReplicateEnsemble("A1", "A"+std::to_string(i));

    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count: " << NN->getNeuronCount() << std::endl;
    std::cout << std::endl;

    // load the images
    auto T = getTimestampMS();
    std::vector<BMPImage> images;
    for (int b = 1; b <= LEARNING_COUNT; b++) {
        auto image = doReadBMP(IMAGES_LEARNING_PATH+std::to_string(b)+".bmp");
        images.push_back(image);
        if (image.size() != 128*128) {
            std::cout << "Error loading image " << b << ".bmp" << std::endl;
            return 1;
        }
    }
    T = getTimestampMS() - T;
    auto input = doBuildInputVector(images);
    std::cout << "Images load done [" << T << " ms]" << std::endl;

    // teach the neural network
    T = getTimestampMS();
    NN -> doLearn(input);
    T = getTimestampMS() - T;
    std::cout << "Learning done [" << T << " ms]" << std::endl;

    // recognize the images
    for (int b = 1; b <= TEST_COUNT; b++) {
        std::cout << "===========================" << std::endl;
        auto image = doReadBMP(IMAGES_TESTING_PATH+std::to_string(b)+".bmp");
        std::vector<BMPImage> rimages;
        for (int i = 1; i <= TEST_COUNT; i++) rimages.push_back(image);
        auto rinput = doBuildInputVector(rimages);

        T = getTimestampMS();
        NN -> doRecognise(rinput);
        auto patterns = NN -> doComparePatterns();
        T = getTimestampMS() - T;

        std::cout << "Recognition done [" << T << " ms]" << std::endl;
        std::cout << std::endl;
        std::cout << "Difference for outputs:" << std::endl;
        for (int i = 0; i < patterns.size(); i++) std::cout << (i+1) << ". " << patterns[i] << std::endl;
    }
}
