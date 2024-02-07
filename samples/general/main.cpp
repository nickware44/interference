/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     22.01.24
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <iomanip>
#include <vector>
#include <iostream>
#include <indk/system.h>
#include <indk/neuralnet.h>
#include <fstream>
#include "bmp.hpp"
#include "general.h"

#define DEFINITIONS_COUNT 6

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

//
//
//
//bool doReceiveResponse(indk::NeuralNet *NN, const std::vector<std::vector<float>>& encoded) {
//    bool found = false;
//    std::vector<std::vector<float>> marks;
//
//    for (int r = 0; r < encoded.size(); r++) {
//        marks.emplace_back();
//        for (int j = 0; j < DEFINITIONS_COUNT; j++) {
//            if (j == (int)encoded[r][1]) marks.back().push_back(encoded[r][0]);
//            else marks.back().push_back(0);
//        }
//    }
//
//    NN -> doRecognise(marks, true, {"SPACE_E1", "SPACE_E2", "SPACE_E3", "SPACE_E4", "SPACE_E5"});
//    auto patterns = NN -> doComparePatterns( "CONTEXT");
//    auto r = std::min_element(patterns.begin(), patterns.end());
//    if (r != patterns.end() && *r >= 0 && *r < 10e-6) found = true;
//
//    return found;
//}
//
//
//
//void doProcessTextSequence(indk::NeuralNet *NN, const std::string& sequence, int &space, const std::array<std::string, DEFINITIONS_COUNT>& definitions) {
//    // parse sequence
//    std::vector<std::vector<float>> encoded;
//    bool qflag = sequence.back() == '?';
//    if (!qflag) return;
//
//    doRecognizeInput(NN, encoded, sequence, 0);
//    auto response = doReceiveResponse(NN, encoded);
//    std::cout << std::setw(50) << std::left << sequence;
//    if (response)
//        std::cout << " [ YES ]" << std::endl;
//    else
//        std::cout << " [ NO  ]" << std::endl;
//}


int main() {
    // load the neural network structure from file
    General general("structures/structure.json");

    // load the rules from text file
    general.doLoadRules( "texts/rules.txt");

    // learn the vocabulary from text file
    general.doLearnVocabulary("texts/vocab.txt");

    // learn the images from files
    general.doLearnVisuals({"images/mug.bmp", "images/jar.bmp", "images/duck.bmp"});


    auto T = getTimestampMS();

    // creating contexts
    general.doCreateContext({General::TypeText("The color can be black, gray, white, orange and blue.")});

    general.doCreateContext({General::TypeText("The cat siting on the table."
                                                        "The cat is black and the table is wooden."
                                                        "Blue light falls from the window."
                                                        "The cat is also half blue."
                                                        "The cat is alien."
                                                        "Other aliens are coming for the cat.")});
//    std::cout << std::endl;
//
//    // checking
//    doProcessTextSequence(NN, "Is the cat gray?", space);
//    doProcessTextSequence(NN, "Is the cat black?", space);
//    doProcessTextSequence(NN, "Is the cat blue?", space);
//    doProcessTextSequence(NN, "Is the table wooden?", space);
//    doProcessTextSequence(NN, "Is the table black?", space);
//    doProcessTextSequence(NN, "Is the cat lying on the table?", space);
//    doProcessTextSequence(NN, "Is the cat siting on the chair?", space);
//    doProcessTextSequence(NN, "Is the cat lying under the table?", space);
//    doProcessTextSequence(NN, "Is the cat siting on the table?", space);
//    doProcessTextSequence(NN, "Is the light falls from the monitor?", space);
//    doProcessTextSequence(NN, "Is the light falls from the window?", space);
//    doProcessTextSequence(NN, "Is the cat alien?", space);
//    doProcessTextSequence(NN, "Are the other aliens coming?", space);
//    doProcessTextSequence(NN, "Are the other aliens coming for human?", space);
//    doProcessTextSequence(NN, "Are the other aliens coming for the cat?", space);
//    std::cout << std::endl;
//
//    doInputWave(NN, {"_SPACE_1", "NV-2", "NV-3", "NV-4"});
    general.doInterlinkDebug();

    T = getTimestampMS() - T;
    std::cout << "Done in " << T << " ms" << std::endl;
    return 0;
}
