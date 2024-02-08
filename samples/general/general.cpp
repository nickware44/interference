/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     07.02.24
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "general.h"
#include "bmp.hpp"

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

General::TypedData General::TypeText(const std::string& text) {
    return {text, TypedDataText};
}

General::TypedData General::TypeVisual(const std::string& path) {
    return {path, TypedDataVisual};
}

std::vector<std::string> General::doLoadVocabulary(const std::string& path) {
    std::vector<std::string> vocab;

    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Can't load the vocabulary file" << std::endl;
    }
    while (!f.eof()) {
        std::string rstr;
        getline(f, rstr);

        vocab.push_back(rstr);
    }

    return vocab;
}

bool General::doCheckRule(const std::vector<std::string>& rule) {
    std::string rstr;
    for (int i = 0; i < rule.size(); i++) {
        rstr.append(rule[i]);
        if (i < rule.size()-1) rstr.append(";");
    }
    std::cout << rstr << std::endl;
    auto ne = std::find_if(Rules.begin(), Rules.end(), [rstr](const std::string& e){
        return e == rstr;
    });

    return ne != Rules.end();
}

void General::doInputWave(const std::vector<std::string>& names) {
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

void General::doRecognizeInput(std::vector<std::vector<float>> &encoded, const std::string& sequence, int type) {
    auto words = doStrSplit(sequence, " ", true);

    // recognize sequence
    for (auto &word: words) {
        if (word.back() == ',') {
            word = word.substr(0, word.size()-1);
        }
        std::vector<std::vector<float>> data;

        for (const auto& ch: word) {
            data.emplace_back();
            data.back().push_back(ch);
        }

        auto Y = NN -> doRecognise(data, true, {"ET"});

        for (int i = 0; i < Definitions.size(); i++) {
            if (Y.size() > i && Y[i] > 0) {
                encoded.push_back({Y[i], static_cast<float>(i)});
            }
        }
    }
}

void General::doCreateContextSpace(const std::vector<std::vector<float>>& encoded) {
    enum {STATE, OBJECT, PROCESS, PLACE, PROPERTY};
    auto l0 = NN -> doReplicateNeuron("_SPACE_INIT", "_SPACE_"+std::to_string(Space)+"_L0", true);
    auto l2 = NN -> doReplicateNeuron("_SPACE_INIT", "_SPACE_"+std::to_string(Space)+"_L2", false);
    if (!l0 || !l2) return;

    std::vector<std::pair<float, std::string>> added;

    l0 -> setLambda(1);
    l2 -> setLambda(3);
    NN -> doIncludeNeuronToEnsemble(l0->getName(), "CONTEXT");
    l0 -> doReset();
    l2 -> doReset();

//    std::cout << encoded.size() << std::endl;
    std::string fname;
    int start = 0;
    for (int r = 0; r < encoded.size(); r++) {
        if (encoded[r][0] == -1) {
//            std::cout << ".";
            start = r+1;
            continue;
        }
        auto code = encoded[r][0];
        auto ne = std::find_if(added.begin(), added.end(), [code](const std::pair<float, std::string>& e){
            return std::fabs(e.first-code) < 10e-6;
        });

        indk::Neuron *l1;
        if (ne == added.end()) {
            l0 -> doCreateNewScope();
            l0 -> doPrepare();

            l1 = NN -> doReplicateNeuron("_SPACE_INIT", "_SPACE_"+std::to_string(Space)+"_"+std::to_string(r), false);
            l1 -> doReplaceEntryName("ES", l0->getName());
            added.emplace_back(code, l1->getName());
            l0 -> doLinkOutput(l1->getName());

            if (fname.empty()) {
                l2 -> doReplaceEntryName("ES", l1->getName());
                fname = l1->getName();
            } else
                l2 -> doCopyEntry(fname, l1->getName());
            l1 -> doLinkOutput(l2->getName());

            l0 -> doSignalSendEntry("ES", encoded[r][0], l0->getTime());

            l1 -> setLambda(1);
            l1 -> setk3(100);
            l1 -> doReset();
            l1 -> doCreateNewScope();
            l1 -> doSignalSendEntry(l0->getName(), encoded[r][0], l1->getTime());
            std::cout << "Y0: " << l1->doSignalReceive().second << std::endl;

            l2 -> doCreateNewScope();
            l2 -> doPrepare();
            std::cout << "Yx: " << l1->doSignalReceive().second << std::endl;
            l2 -> doSignalSendEntry(l1->getName(), l1->doSignalReceive().second,  l2->getTime());
            for (const auto &ew: l2->getWaitingEntries()) {
                l2 -> doSignalSendEntry(NN->getNeuron(ew)->getName(), 0, l2->getTime());
            }
        } else {
            l1 = NN -> getNeuron(ne->second);
            if (!l1) continue;
        }

        if (r-1 >= 0 && encoded[r-1][0] != -1) {
//            std::cout << start << " " << encoded[r-1][0] << std::endl;
            l1 -> doCreateNewScope();
            l1 -> doPrepare();

            std::vector<float> values;
            values.emplace_back(encoded[r][0]);

            for (int i = r-1; i >= start; i--) {
                auto checked = doCheckRule({Definitions[encoded[i][1]], Definitions[encoded[r][1]]});
                if (checked) {
//                    std::cout << encoded[i][0] << " " << encoded[r][1] << std::endl;
                    values.emplace_back(encoded[i][0]);
                }
            }

            for (int i = values.size()-1; i >= 0; i--) {
//                std::cout << values[i] << std::endl;
                l1 -> doSignalSendEntry(l0->getName(), values[i], l1->getTime());
                std::cout << "Y1: " << l1->doSignalReceive().second << std::endl;
            }

            l2 -> doCreateNewScope();
            l2 -> doPrepare();
            std::cout << "Yx: " << l1->doSignalReceive().second << std::endl;
            l2 -> doSignalSendEntry(l1->getName(), l1->doSignalReceive().second,  l2->getTime());
            for (const auto &ew: l2->getWaitingEntries()) {
                l2 -> doSignalSendEntry(NN->getNeuron(ew)->getName(), 0, l2->getTime());
            }

        }
//        std::cout << " " << Definitions[encoded[r][1]];
//        std::cout <<  encoded[r][0] << " " << encoded[r][1] << std::endl;
    }
    std::cout << std::endl;

    l0 -> doFinalize();
    l0 -> setOutputMode(indk::Neuron::OutputModes::OutputModeLatch);

    for (auto &a: added) {
        auto n = NN -> getNeuron(a.second);
        if (n) {
            n -> doFinalize();
        }
    }

    l2 -> doFinalize();
}




General::General(const std::string &path) {
    Space = 0;
    NN = new indk::NeuralNet(path);
//    indk::System::setVerbosityLevel(2);
    NN -> doInterlinkInit(4408, 1);
    NN -> doPrepare();

    std::cout << "Threads     : " << indk::System::getComputeBackendParameter() << std::endl;
    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << std::endl;
}

void General::doLearnVocabulary(const std::string& path) {
    // load the vocabulary
    auto vocab = doLoadVocabulary(path);

    // learn the vocabulary
    for (int i = 1; i <= vocab.size(); i++) {
        const auto& item = vocab[i-1];

        std::string word = item.substr(0, item.find(';'));
        std::string definition = item.substr(item.find(';')+1);

        auto n = NN -> getNeuron(definition);
        if (!n) return;

        n -> doPrepare();
        for (const auto& ch: word) {
            n -> doSignalSendEntry("ET", (float)ch, n->getTime());
        }
        n -> doCreateNewScope();
    }

    for (auto &d: Definitions) {
        auto n = NN -> getNeuron(d);
        n -> setOutputMode(indk::Neuron::OutputModes::OutputModeLatch);
        n -> doFinalize();
    }
}

void General::doLearnVisuals(const std::vector<std::string>& paths) {
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

void General::doLoadRules(const std::string& path) {
    Rules.clear();

    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Can't load the rules file" << std::endl;
    }
    while (!f.eof()) {
        std::string rstr;
        getline(f, rstr);

        Rules.push_back(rstr);
    }
}

void General::doCreateContext(const std::vector<TypedData> &sequence) {
    std::vector<std::vector<float>> encoded;

    for (const auto &seq: sequence) {
        std::cout << seq.first << std::endl;

        if (seq.second == General::TypedDataText) {
            auto sentences = doStrSplit(seq.first, ".", true);
            for (const auto &str: sentences) {
                doRecognizeInput(encoded, str, seq.second);
                encoded.push_back({-1, -1});
            }
        } else if (seq.second == General::TypedDataVisual) {
            doRecognizeInput(encoded, seq.first, seq.second);
            encoded.push_back({-1, -1});
        }
    }

    doCreateContextSpace(encoded);
    Space++;
}

void General::doInterlinkDebug() {
    NN -> doInterlinkSyncStructure();
    NN -> doInterlinkSyncData();
}

General::~General() {
    delete NN;
}
