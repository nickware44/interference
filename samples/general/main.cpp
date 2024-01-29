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

#define DEFINITIONS_COUNT 5

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

void doLearnVocabulary(indk::NeuralNet *NN,
                       const std::array<std::string, 5>& definitions,
                       const std::vector<std::string>& vocab) {
    auto n = NN -> getNeuron("N1");
    if (!n) return;

    n -> setOutputMode(indk::Neuron::OutputModes::OutputModeLatch);

    // learn the vocabulary
    for (int i = 1; i <= vocab.size(); i++) {
        const auto& item = vocab[i-1];

        std::string word = item.substr(0, item.find(';'));
//        std::string definition = item.substr(item.find(';')+1);
//        std::string source, destination;

//        for (int d = 0; d < definitions.size(); d++) {
//            if (definition == definitions[d]) {
//                source = "N"+std::to_string(d+1);
//                destination = "N"+std::to_string(definitions.size()+i);
//                NN -> doReplicateNeuron(source, destination, true);
//                break;
//            }
//        }

//        NN -> doReplicateNeuron("ND-1", "ND-"+std::to_string(i+1), true);
//        auto n = NN -> getNeuron(destination);


//        NN -> doIncludeNeuronToEnsemble(n->getName(), "TEXT");
//        auto dn = NN -> getNeuron(n->getLinkOutput()[0]);
//        dn -> doCopyEntry(source, n->getName());
//        n -> doLinkOutput(dn->getName());
        n -> doPrepare();
        for (const auto& ch: word) {
            n -> doSignalSendEntry("ET", (float)ch, n->getTime());
        }
        n -> doCreateNewScope();
//        n -> setOutputMode(indk::Neuron::OutputModes::OutputModeLatch);
    }
    n -> doFinalize();
}

auto doLearnVisuals(indk::NeuralNet *NN, const std::vector<std::string>& paths) {
    for (int p = 0; p < paths.size(); p++) {
        auto image = doReadBMP(paths[p]);
        auto input = doBuildImageInputVector({image});
        auto destination = "NV-"+std::to_string(p+2);
        auto n = NN -> doReplicateNeuron("NV-1", destination, true);
        NN -> doIncludeNeuronToEnsemble(n->getName(), "VISION");

        for (auto& in: input) {
            n -> doSignalSendEntry("EV1", in[0], n->getTime());
            n -> doSignalSendEntry("EV2", in[1], n->getTime());
            n -> doSignalSendEntry("EV3", in[2], n->getTime());
            n -> doSignalSendEntry("EV4", in[3], n->getTime());
            n -> doSignalSendEntry("EV5", in[4], n->getTime());
            n -> doSignalSendEntry("EV6", in[5], n->getTime());
        }
        n -> doFinalize();
        NN -> doAddNewOutput(destination);
    }
}

auto doRecognizeInput(indk::NeuralNet *NN, const std::string& sequence, int type) {
    std::vector<std::vector<float>> encoded;
    auto stripped = sequence.substr(0, sequence.size()-1);
    auto words = doStrSplit(stripped, " ", true);

    // recognize sequence
    for (const auto &word: words) {
        std::vector<std::vector<float>> data;

        for (const auto& ch: word) {
            data.emplace_back();
            data.back().push_back(ch);
        }

        auto Y = NN -> doRecognise(data, true, {"ET"});
//        std::cout << Y[0] << std::endl;
        if (!Y.empty() && Y[0] > 0) encoded.push_back({Y[0], static_cast<float>(0)});
    }
    return encoded;
}

void doCreateContextSpace(indk::NeuralNet *NN, const std::vector<std::vector<float>>& encoded, int space) {
    NN -> doReplicateNeuron("_SPACE_INIT", "_SPACE_"+std::to_string(space), true);
    auto n = NN -> getNeuron("_SPACE_"+std::to_string(space));
    if (!n) return;

    n -> setLambda(1);
    NN -> doIncludeNeuronToEnsemble(n->getName(), "CONTEXT");
    n -> doReset();

    std::cout << encoded.size() << std::endl;

    for (int r = 0; r < encoded.size(); r++) {
        n -> doCreateNewScope();
        n -> doPrepare();
        auto dn = NN -> doReplicateNeuron("_SPACE_INIT", "_SPACE_"+std::to_string(space)+"_"+std::to_string(r), false);
        dn -> doReplaceEntryName("ES", n->getName());
        n -> doLinkOutput(dn->getName());
        n -> doSignalSendEntry("ES", encoded[r][0], n->getTime());
    }
    n -> doFinalize();
}

bool doReceiveResponse(indk::NeuralNet *NN, const std::vector<std::vector<float>>& encoded) {
    bool found = false;
    std::vector<std::vector<float>> marks;

    for (int r = 0; r < encoded.size(); r++) {
        marks.emplace_back();
        for (int j = 0; j < DEFINITIONS_COUNT; j++) {
            if (j == (int)encoded[r][1]) marks.back().push_back(encoded[r][0]);
            else marks.back().push_back(0);
        }
    }

    NN -> doRecognise(marks, true, {"SPACE_E1", "SPACE_E2", "SPACE_E3", "SPACE_E4", "SPACE_E5"});
    auto patterns = NN -> doComparePatterns( "CONTEXT");
    auto r = std::min_element(patterns.begin(), patterns.end());
    if (r != patterns.end() && *r >= 0 && *r < 10e-6) found = true;

    return found;
}

void doProcessTextSequence(indk::NeuralNet *NN, std::string sequence, int &space) {
    // parse sequence
    bool qflag = sequence.back() == '?';
    auto encoded = doRecognizeInput(NN, sequence, 0);

    if (!qflag) {
        // create new space in the context
        std::cout << sequence << std::endl;
        doCreateContextSpace(NN, encoded, space);
        space++;
    } else {
        // check info in the context if this is a question
        auto response = doReceiveResponse(NN, encoded);
        std::cout << std::setw(50) << std::left << sequence;
        if (response)
            std::cout << " [ YES ]" << std::endl;
        else
            std::cout << " [ NO  ]" << std::endl;
    }
}

auto doInputWave(indk::NeuralNet *NN, const std::vector<std::string>& names) {
    for (const auto& name: names) {
        auto n = NN -> getNeuron(name);
        if (!n) continue;

        n -> doPrepare();
        for (int i = 0; i < n->getReceptorsCount(); i++)
            n -> getReceptor(i) -> setPos(n->getReceptor(i)->getPos());

        auto we = n -> getWaitingEntries();
        for (const auto &e: we) {
            n -> doSignalSendEntry(e, 100, n->getTime());
        }
        n -> doFinalize();
        std::cout << name << " \t" << n->doSignalReceive().second << std::endl;
    }
}

int main() {
    constexpr char STRUCTURE_PATH[128] = "structures/structure.json";
    constexpr char VOCAB_PATH[128] = "texts/vocab.txt";
    std::array<std::string, DEFINITIONS_COUNT> definitions = {"STATE", "OBJECT", "PROCESS", "PLACE", "PROPERTY"};

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

//    for (const auto& d: definitions) {
//        NN -> doIncludeNeuronToEnsemble(d, "DEFINITION");
//        NN -> getNeuron(d) -> setLambda(0.1);
//    }

    int space = 1;
    auto T = getTimestampMS();
    doLearnVocabulary(NN, definitions, vocab);
    doLearnVisuals(NN, {"images/mug.bmp", "images/jar.bmp", "images/duck.bmp"});

    // creating context
    doProcessTextSequence(NN, "The cat siting on the table.", space);
    doProcessTextSequence(NN, "The cat is black and the table is wooden.", space);
    doProcessTextSequence(NN, "Blue light falls from the window.", space);
    doProcessTextSequence(NN, "The cat is also half blue.", space);
    doProcessTextSequence(NN, "The cat is alien.", space);
    doProcessTextSequence(NN, "Other aliens are coming for the cat.", space);
    std::cout << std::endl;
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

    NN -> doInterlinkSyncStructure();
    NN -> doInterlinkSyncData();

    T = getTimestampMS() - T;
    std::cout << "Done in " << T << " ms" << std::endl;
    return 0;
}
