/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     07.02.24
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_GENERAL_H
#define INTERFERENCE_GENERAL_H


#include <array>
#include <string>
#include "indk/neuralnet.h"

class General {
private:
    std::array<std::string, 6> Definitions = {"STATE", "OBJECT", "PROCESS", "PLACE", "PROPERTY", "LOGIC"};
    typedef std::pair<std::string, int> TypedData;
    typedef std::tuple<float, int, std::vector<std::string>> EncodeData;
    enum {
        TypedDataText,
        TypedDataVisual
    };

    indk::NeuralNet *NN;
    std::vector<std::string> Rules;
    int Space;

    static std::vector<std::string> doLoadVocabulary(const std::string& path);
    bool doCheckRule(const std::vector<std::string>& rule);
    void doInputWave(const std::vector<std::string>& names);
    void doRecognizeInput(std::vector<EncodeData> &encoded, const std::string& sequence, int type);
    void doCreateContextSpace(std::vector<EncodeData>& encoded);
public:
    static TypedData TypeText(const std::string&);
    static TypedData TypeVisual(const std::string&);

    General() = default;
    explicit General(const std::string& path);

    void doLearnVocabulary(const std::string& path);
    void doLearnVisuals(const std::vector<std::string>& paths);
    void doLoadRules(const std::string& path);
    void doCreateContext(const std::vector<TypedData>& sequence);
    void doInterlinkDebug();
    ~General();
};


#endif //INTERFERENCE_GENERAL_H
