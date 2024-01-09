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
#include "indk/profiler.h"

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

int doProcessTextSequence(indk::NeuralNet *NN,
                       const std::array<std::string, 5>& definitions,
                       const std::vector<std::vector<std::string>>& destinations,
                       std::string sequence,
                       int &space) {
    NN -> doReplicateNeuron("_SPACE_INIT", "_SPACE_"+std::to_string(space), true);
    auto n = NN -> getNeuron("_SPACE_"+std::to_string(space));
    if (!n) return -1;

    // creating new space in the context
    n -> setLambda(1);
    std::vector<std::vector<float>> related;

    bool qflag = sequence.back() == '?';
    auto stripped = sequence.substr(0, sequence.size()-1);
    auto words = doStrSplit(stripped, " ", true);

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

    n -> doReset();
    indk::Position *pos = nullptr;
    for (auto & r : related) {
        n -> doCreateNewScope();
        n -> doPrepare();
        if (pos) n -> getReceptor(0) -> getPos() -> setPosition(pos);
        for (int j = 0; j < definitions.size(); j++) {
            if (j == (int)r[1]) {
                n -> doSignalSendEntry("SPACE_E"+std::to_string(j+1), r[0], n->getTime());
            } else {
                n -> doSignalSendEntry("SPACE_E"+std::to_string(j+1), 0, n->getTime());
            }
        }
        pos = n -> getReceptor(0) -> getPos();
    }
    n -> doFinalize();

    // check info in the context if this is a question
    if (qflag) {
        int found = 0;
        auto vectors = n -> getReceptor(0) -> getReferencePosScopes();
        std::vector<float> marks;
        for (auto v: vectors) {
            auto d = indk::Position::getDistance(*v, indk::Position(100, 3));
            marks.push_back(d);
        }

        for (int s = 1; s < space; s++) {
            auto sn = NN -> getNeuron("_SPACE_"+std::to_string(s));
            if (!sn) return -1;
            int mfound = 0;

            auto svectors = sn -> getReceptor(0) -> getReferencePosScopes();
            for (auto sv: svectors) {
                auto d = indk::Position::getDistance(*sv, indk::Position(100, 3));
                if (marks.size() > mfound) {
                    if (std::fabs(marks[mfound]-d) < 10e-4) {
                        mfound++;
                    } else if (std::fabs(marks[0]-d) < 10e-4) {
                        mfound = 1;
                    } else {
                        mfound = 0;
                    }
                }

                if (mfound == marks.size()) {
                    found = 1;
                    break;
                }
            }
            if (found) break;
        }

        std::cout << std::setw(50) << std::left << sequence;
        if (found)
            std::cout << " [ YES ]" << std::endl;
        else
            std::cout << " [ NO  ]" << std::endl;

        NN -> doDeleteNeuron("_SPACE_"+std::to_string(space));
        return found;
    }

    std::cout << sequence << std::endl;
    space++;
    return 0;
}

int main() {
    constexpr char STRUCTURE_PATH[128] = "structures/structure.json";
    constexpr char VOCAB_PATH[128] = "texts/vocab.txt";
    std::array<std::string, 5> definitions = {"STATE", "OBJECT", "PROCESS", "PLACE", "PROPERTY"};

    // load vocabulary from text file
    auto vocab = doLoadVocabulary(VOCAB_PATH);

    // load neural network structure from file
    auto NN = new indk::NeuralNet(STRUCTURE_PATH);
    NN -> doInterlinkInit(4408, 2);
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
