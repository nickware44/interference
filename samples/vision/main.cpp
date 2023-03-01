/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     30.01.23
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

std::vector<std::vector<double>> doBuildInputVector(std::vector<BMPImage> &images) {
    std::vector<std::vector<double>> input;
    for (int d = 0; d < images[0].size(); d+=2) {
        input.emplace_back();
        for (int i = 0; i < images.size(); i++) {
            for (int s = 0; s < 2; s++) {
                auto rgbn = std::vector<double>({images[i][d+s][0]/255., images[i][d+s][1]/255., images[i][d+s][2]/255.});
                auto HSI = RGB2HSI(rgbn[0], rgbn[1], rgbn[2]);
                input.back().emplace_back(HSI[0]/(2*M_PI));
                input.back().emplace_back(HSI[1]);
                input.back().emplace_back(HSI[2]);
            }
        }
    }
    return input;
}

void doLog(const std::string& element, uint64_t time, double speed, bool endl = true) {
    std::stringstream s;
    s << time << "ms";
    if (speed > 0) s << ", " << std::setprecision(2) << std::fixed << speed << " mbit/s";
    std::cout << "[" << std::setw(22) << std::left << std::setfill('.') << s.str() << "] " << std::setw(32) << std::setfill(' ') << element << "done ";
    if (endl) std::cout << std::endl;
}

int main() {
    inn::System::setComputeBackend(inn::System::ComputeBackends::Multithread, 2);
    constexpr uint8_t TEACH_COUNT = 10;
    constexpr uint8_t TEST_COUNT = 10;
    constexpr uint8_t TEST_ELEMENTS = 10;
    constexpr uint16_t IMAGE_SIZE = 128*128;
    constexpr char STRUCTURE_PATH[128] = "../samples/vision/structure.json";
    constexpr char IMAGES_TEACHING_PATH[128] = "../samples/vision/images/learn/";
    constexpr char IMAGES_TESTING_PATH[128] = "../samples/vision/images/test/";

    // load neural network structure from file
    std::ifstream structure(STRUCTURE_PATH);
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);

    // replicate neurons for classification
    for (int i = 2; i <= TEACH_COUNT; i++) NN -> doReplicateEnsemble("A1", "A"+std::to_string(i), true);

    std::cout << "Threads     : " << inn::System::getComputeBackendParameter()+1 << std::endl;
    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count: " << NN->getNeuronCount() << std::endl;
    std::cout << std::endl;

    // load the images
    auto T = getTimestampMS();
    std::vector<BMPImage> images;
    for (int b = 1; b <= TEACH_COUNT; b++) {
        auto image = doReadBMP(IMAGES_TEACHING_PATH+std::to_string(b)+".bmp");
        images.push_back(image);
        if (image.size() != IMAGE_SIZE) {
            std::cout << "Error loading image " << b << ".bmp" << std::endl;
            return 1;
        }
    }
    T = getTimestampMS() - T;
    auto input = doBuildInputVector(images);
    doLog("Loading images", T, 0);

    // teach the neural network
    T = getTimestampMS();
    NN -> doLearn(input);
    T = getTimestampMS() - T;

    // Compute speed
    auto S = (IMAGE_SIZE*TEACH_COUNT*24./1024/1024)*1000 / T;
    doLog("Teaching neural network", T, S);

    // recognize the images
    float rcount = 0;
    for (int b = 1; b <= TEST_COUNT; b++) {
        for (int e = 1; e <= TEST_ELEMENTS; e++) {
            std::string name = std::to_string(b)+"-"+std::to_string(e)+".bmp";
            auto image = doReadBMP(IMAGES_TESTING_PATH+name);
            std::vector<BMPImage> rimages;
            for (int i = 1; i <= TEST_COUNT; i++) rimages.push_back(image);
            auto rinput = doBuildInputVector(rimages);

            T = getTimestampMS();
            NN -> doRecognise(rinput);
            T = getTimestampMS() - T;

            // Compute speed
            S = (IMAGE_SIZE*24./1024/1024)*1000 / T;
            doLog("Recognizing "+std::to_string(b)+"-"+std::to_string(e)+".bmp", T, S, false);

            auto patterns = NN -> doComparePatterns();
            auto r = std::max_element(patterns.begin(), patterns.end());
            if (std::distance(patterns.begin(), r) == b-1) {
                std::cout << "[RECOGNIZED]" << std::endl;
                rcount++;
            } else {
                std::cout << "[NOT RECOGNIZED]" << std::endl;
                // Uncomment to print the response of output neurons to the input data (response - values [0, 1], 0 - minimum response, 1 - maximum response)
                // std::cout << "Difference for outputs:" << std::endl;
                // for (int i = 0; i < patterns.size(); i++) std::cout << (i+1) << ". " << patterns[i] << std::endl;
            }
        }
    }

    std::cout << std::endl;
    std::cout << "================================== SUMMARY ==================================" << std::endl;
    std::cout << "Recognition accuracy: " << rcount/(TEST_COUNT*TEST_ELEMENTS) << " (" << rcount << "/" << TEST_COUNT*TEST_ELEMENTS << ")" << std::endl;
}
