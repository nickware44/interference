/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     31.10.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <iomanip>
#include <utility>
#include <vector>
#include <iostream>
#include <indk/system.h>
#include <indk/neuralnet.h>
#include <fstream>
#include "bmp.hpp"

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

inline std::vector<std::string> doStrSplit(std::string str, const std::string& div, bool last) {
    size_t pos;
    std::vector<std::string> R;
    std::string token;
    while ((pos = str.find(div)) != std::string::npos) {
        token = str.substr(0, pos);
        R.push_back(token);
        str.erase(0, pos+div.length());
    }
    if (last) R.push_back(str);
    return R;
}

auto doLoadVocabulary(const std::string& path) {
    std::vector<std::string> data;

    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Can't load the vocabulary file" << std::endl;
        return std::vector<std::string>();
    }
    while (!f.eof()) {
        std::string rstr;
        getline(f, rstr);

        data.push_back(rstr);
    }

    return data;
}

std::vector<std::vector<float>> doBuildImageInputVector(std::vector<BMPImage> images) {
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
        }
    }
    return input;
}

auto doLearnVocabulary(indk::NeuralNet *NN,
                       const std::array<std::string, 5>& definitions,
                       const std::vector<std::string>& vocab) {
    int d;
    std::vector<std::vector<std::string>> destinations;
    std::array<int, 5> stats = {0};

    // learn the vocabulary
    for (int i = 1; i <= vocab.size(); i++) {
        std::vector<std::vector<float>> input;
        const auto& item = vocab[i-1];

        std::string word = item.substr(0, item.find(';'));
        std::string definition = item.substr(item.find(';')+1);
        std::string destination;

        for (d = 0; d < definitions.size(); d++) {
            if (definition == definitions[d]) {
                destination = "N"+std::to_string(d+1)+"-"+std::to_string(stats[d]);
                destinations.push_back({destination, word, std::to_string(d)});
                NN -> doReplicateNeuron("N"+std::to_string(d+1), destination, true);
                stats[d]++;
                break;
            }
        }

        auto n = NN -> getNeuron(destination);
        if (!n) break;
        NN -> doAddNewOutput(destination);

        for (const auto& ch: word) {
            input.emplace_back();
            input.back().push_back(ch);
            n -> doSignalSendEntry("E"+std::to_string(d+1), (float)ch, n->getTime());
        }
        n -> doFinalize();
    }
    return destinations;
}

auto doLearnVisual(indk::NeuralNet *NN,
                   const std::array<std::string, 5>& definitions,
                   const std::vector<std::string>& paths) {
    for (int p = 0; p < paths.size(); p++) {
        auto image = doReadBMP(paths[p]);
        auto input = doBuildImageInputVector({image});
        auto destination = "NV-"+std::to_string(p+2);
        auto n = NN -> doReplicateNeuron("NV-1", destination, true);

        for (auto& in: input) {
            n -> doSignalSendEntry("EV-1", in[0], n->getTime());
            n -> doSignalSendEntry("EV-2", in[1], n->getTime());
            n -> doSignalSendEntry("EV-3", in[2], n->getTime());
            n -> doSignalSendEntry("EV-4", in[3], n->getTime());
            n -> doSignalSendEntry("EV-5", in[4], n->getTime());
            n -> doSignalSendEntry("EV-6", in[5], n->getTime());
        }
        n -> doFinalize();

//        images.push_back(image);
//        if (image.size() != IMAGE_SIZE) {
//            std::cout << "Error loading image " << b << ".bmp" << std::endl;
//            return 1;
//        }
    }
//    auto input = doBuildImageInputVector(images);
//    NN -> doLearn(input);
}

int doProcessTextSequence(indk::NeuralNet *NN,
                       const std::array<std::string, 5>& definitions,
                       const std::vector<std::vector<std::string>>& destinations,
                       std::string sequence,
                       int &space) {
    std::vector<std::vector<float>> related;

    // parse sequence
    bool qflag = sequence.back() == '?';
    auto stripped = sequence.substr(0, sequence.size()-1);
    auto words = doStrSplit(stripped, " ", true);

    // recognize sequence
    for (const auto &word: words) {
        std::vector<std::vector<float>> data;

        for (const auto& ch: word) {
            data.emplace_back();
            for (int i = 0; i < definitions.size(); i++)
                data.back().push_back(ch);
        }
        auto Y = NN -> doRecognise(data, true, {"E1", "E2", "E3", "E4", "E5"});
        auto patterns = NN -> doComparePatterns();

        for (int i = 0; i < patterns.size(); i++) {
            if (patterns[i] == 0) {
                related.push_back({Y[i], static_cast<float>(std::stoi(destinations[i][2]))});
            }
        }
    }

    if (!qflag) {
        // create new space in the context

        NN -> doReplicateNeuron("_SPACE_INIT", "_SPACE_"+std::to_string(space), true);
        auto n = NN -> getNeuron("_SPACE_"+std::to_string(space));
        if (!n) return -1;

        n -> setLambda(1);
        NN -> doIncludeNeuronToEnsemble(n->getName(), "CONTEXT");
        n -> doReset();

        int nstart = 0;
        while (nstart < related.size()) {
            indk::Position *pos = nullptr;

            for (int r = nstart; r < related.size(); r++) {
                n -> doCreateNewScope();
                n -> doPrepare();
                if (pos) n -> getReceptor(0) -> getPos() -> setPosition(pos);
                for (int j = 0; j < definitions.size(); j++) {
                    if (j == (int)related[r][1]) {
                        n -> doSignalSendEntry("SPACE_E"+std::to_string(j+1), related[r][0], n->getTime());
                    } else {
                        n -> doSignalSendEntry("SPACE_E"+std::to_string(j+1), 0, n->getTime());
                    }
                }
                pos = n -> getReceptor(0) -> getPos();
            }
            nstart++;
        }
        n -> doFinalize();
    } else {
        // check info in the context if this is a question

        int found = 0;
        std::vector<std::vector<float>> marks;

        for (int r = 0; r < related.size(); r++) {
            marks.emplace_back();
            for (int j = 0; j < definitions.size(); j++) {
                if (j == (int)related[r][1]) marks.back().push_back(related[r][0]);
                else marks.back().push_back(0);
            }
        }

        NN -> doRecognise(marks, true, {"SPACE_E1", "SPACE_E2", "SPACE_E3", "SPACE_E4", "SPACE_E5"});
        auto patterns = NN -> doComparePatterns( "CONTEXT");
//        for (int i = 0; i < patterns.size(); i++) std::cout << (i+1) << ". " << patterns[i] << std::endl;
        auto r = std::min_element(patterns.begin(), patterns.end());
        if (r != patterns.end() && *r < 10e-4) found = 1;

        std::cout << std::setw(50) << std::left << sequence;
        if (found)
            std::cout << " [ YES ]" << std::endl;
        else
            std::cout << " [ NO  ]" << std::endl;

        return found;
    }

    std::cout << sequence << std::endl;
    space++;
    return 0;
}

int main() {
    constexpr char STRUCTURE_PATH[128] = "structures/structure.json";
    constexpr char VOCAB_PATH[128] = "texts/vocab.txt";
    constexpr char IMAGES_PATH[128] = "images/";
    std::array<std::string, 5> definitions = {"STATE", "OBJECT", "PROCESS", "PLACE", "PROPERTY"};

    // load vocabulary from text file
    auto vocab = doLoadVocabulary(VOCAB_PATH);

    // load neural network structure from file
    auto NN = new indk::NeuralNet(STRUCTURE_PATH);
    NN -> doInterlinkInit(4408, 1);
//    indk::System::setVerbosityLevel(2);
    NN -> doPrepare();

    std::cout << "Threads     : " << indk::System::getComputeBackendParameter() << std::endl;
    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << std::endl;

    int space = 1;
    auto T = getTimestampMS();
    auto destinations = doLearnVocabulary(NN, definitions, vocab);
    doLearnVisual(NN, definitions, {"images/1.bmp", "images/2.bmp"});

    // creating context
    doProcessTextSequence(NN, definitions, destinations, "The cat siting on the table.", space);
    doProcessTextSequence(NN, definitions, destinations, "The cat is black and the table is wooden.", space);
    doProcessTextSequence(NN, definitions, destinations, "Blue light falls from the window.", space);
    doProcessTextSequence(NN, definitions, destinations, "The cat is also half blue.", space);
    doProcessTextSequence(NN, definitions, destinations, "The cat is alien.", space);
    doProcessTextSequence(NN, definitions, destinations, "Other aliens are coming for the cat.", space);
    std::cout << std::endl;

    // checking
    doProcessTextSequence(NN, definitions, destinations, "Is the cat gray?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the cat black?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the cat blue?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the table wooden?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the table black?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the cat lying on the table?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the cat siting on the chair?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the cat lying under the table?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the cat siting on the table?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the light falls from the monitor?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the light falls from the window?", space);
    doProcessTextSequence(NN, definitions, destinations, "Is the cat alien?", space);
    doProcessTextSequence(NN, definitions, destinations, "Are the other aliens coming?", space);
    doProcessTextSequence(NN, definitions, destinations, "Are the other aliens coming for human?", space);
    doProcessTextSequence(NN, definitions, destinations, "Are the other aliens coming for the cat?", space);
    std::cout << std::endl;

    NN -> doInterlinkSyncStructure();
    NN -> doInterlinkSyncData();

    T = getTimestampMS() - T;
    std::cout << "Done in " << T << " ms" << std::endl;
    return 0;
}
