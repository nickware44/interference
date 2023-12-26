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
#include <indk/system.h>
#include <indk/neuralnet.h>
#include "bmp.hpp"
#include "indk/profiler.h"

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

std::vector<std::vector<float>> doBuildInputVector(std::vector<BMPImage> images) {
    std::vector<std::vector<float>> input;
    for (int d = 0; d < images[0].size(); d+=2) {
        input.emplace_back();
        for (int i = 0; i < images.size(); i++) {
            for (int s = 0; s < 2; s++) {
                float r = images[i][d+s][0];
                float g = images[i][d+s][1];
                float b = images[i][d+s][2];
                auto rgbn = std::vector<float>({r/255, g/255, b/255});
                auto HSI = RGB2HSI(rgbn[0], rgbn[1], rgbn[2]);
                input.back().emplace_back(HSI[0]/(2*M_PI));
                input.back().emplace_back(HSI[1]);
                input.back().emplace_back(HSI[2]);
            }
            for (int s = 0; s < 2; s++) {
                float r = images[i][d+s][0];
                float g = images[i][d+s][1];
                float b = images[i][d+s][2];
                auto rgbn = std::vector<float>({r/255, g/255, b/255});
                input.back().emplace_back(rgbn[0]);
                input.back().emplace_back(rgbn[1]);
                input.back().emplace_back(rgbn[2]);
            }
        }
    }
    return input;
}

void doLog(const std::string& element, uint64_t time, float speed, bool endl = true) {
    std::stringstream s;
    s << time << "ms";
    if (speed > 0) s << ", " << std::setprecision(2) << std::fixed << speed << " mbit/s";
    std::cout << "[" << std::setw(22) << std::left << std::setfill('.') << s.str() << "] " << std::setw(32) << std::setfill(' ') << element << "done ";
    if (endl) std::cout << std::endl;
}

int main() {
//    indk::System::setComputeBackend(indk::System::ComputeBackends::Multithread, 2);
    constexpr uint8_t TEACH_COUNT = 10;
    constexpr uint8_t TEST_COUNT = 10;
    constexpr uint8_t TEST_ELEMENTS = 10;
    constexpr uint16_t IMAGE_SIZE = 128*128;
    constexpr char STRUCTURE_PATH[128] = "structures/structure2.json";
    constexpr char IMAGES_TEACHING_PATH[128] = "images/learn/";
    constexpr char IMAGES_TESTING_PATH[128] = "images/test/";

    // load neural network structure from file
    std::ifstream structure(STRUCTURE_PATH);
    auto NN = new indk::NeuralNet(STRUCTURE_PATH);
    NN -> setStateSyncEnabled();
    NN -> doInterlinkInit(4408);

//    indk::Profiler::doAttachCallback(NN, indk::Profiler::EventFlags::EventTick, [](indk::NeuralNet *nn) {
//        if (nn->getNeuron("N1")->getTime() >= 8192) {
//            auto p1 = nn->getNeuron("N1")->getReceptor(0)->getPos();
//            auto p2 = nn->getNeuron("N1")->getReceptor(0)->getPosf();
//            std::cout << p1->getPositionValue(0) << " " << p1->getPositionValue(1) << " ";
//            std::cout << p2->getPositionValue(0) << " " << p2->getPositionValue(1) << std::endl;
//        }
//    });

    // replicate neurons for classification
    for (int i = 2; i <= TEACH_COUNT; i++) NN -> doReplicateEnsemble("A1", "A"+std::to_string(i), true);

    std::cout << "Threads     : " << indk::System::getComputeBackendParameter() << std::endl;
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
            auto rinput = doBuildInputVector({image});

            T = getTimestampMS();
            NN -> doRecognise(rinput, "A1");
            T = getTimestampMS() - T;

            // Compute speed
            S = (IMAGE_SIZE*24./1024/1024)*1000 / T;
            doLog("Recognizing "+std::to_string(b)+"-"+std::to_string(e)+".bmp", T, S, false);

            auto patterns = NN -> doComparePatterns(indk::PatternCompareFlags::CompareNormalized);
//            std::vector<float> patterns1, patterns2;
//            for (int i = 0; i < patterns.size(); i++) {
//                if (!(i%2))
//                    patterns1.push_back(patterns[i]);
//                else
//                    patterns2.push_back(patterns[i]);
//            }
            auto r1 = std::max_element(patterns.begin(), patterns.end());
//            auto r2 = std::max_element(patterns2.begin(), patterns2.end());
            if (std::distance(patterns.begin(), r1) == b-1) {
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
    return 0;
}
