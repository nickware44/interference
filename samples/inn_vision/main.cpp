#include <iostream>
#include <thread>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <inn/neuralnet.h>
#include <inn/error.h>
#include <sys/time.h>
#include <bitset>
#include "lodepng.h"

typedef std::tuple<double, double, double> ColorDefinition;
typedef std::tuple<unsigned char, unsigned char, unsigned char> Color;
typedef std::tuple<long, long, long> PosDefinition;
typedef std::tuple<unsigned long, unsigned long, unsigned long, unsigned long, int> ObjectDefinition;
typedef std::vector<PosDefinition> Area;

struct timeval tv1,tv2,dtv;
struct timezone tz;

void time_start() {
    gettimeofday(&tv1, &tz);
}

long time_stop() {
    gettimeofday(&tv2, &tz);
    dtv.tv_sec= tv2.tv_sec -tv1.tv_sec;
    dtv.tv_usec=tv2.tv_usec-tv1.tv_usec;
    if (dtv.tv_usec<0) { dtv.tv_sec--; dtv.tv_usec+=1000000; }
    return dtv.tv_sec*1000+dtv.tv_usec/1000;
}

Color GetColor(int OType) {
    Color C;
    switch (OType) {
        case 0: // orange
            std::get<0>(C) = 250;
            std::get<1>(C) = 100;
            std::get<2>(C) = 4;
            break;
        case 1: // light green
            std::get<0>(C) = 140;
            std::get<1>(C) = 214;
            std::get<2>(C) = 119;
            break;
        case 2: // light purple
            std::get<0>(C) = 190;
            std::get<1>(C) = 190;
            std::get<2>(C) = 255;
            break;
        case 3: // light yellow
            std::get<0>(C) = 254;
            std::get<1>(C) = 200;
            std::get<2>(C) = 140;
            break;
        case 4: // light red
            std::get<0>(C) = 239;
            std::get<1>(C) = 80;
            std::get<2>(C) = 120;
            break;
        case 5: // aquamarine
            std::get<0>(C) = 57;
            std::get<1>(C) = 242;
            std::get<2>(C) = 240;
            break;
        case 6: // green
            std::get<0>(C) = 0;
            std::get<1>(C) = 255;
            std::get<2>(C) = 10;
            break;
        case 7: // purple
            std::get<0>(C) = 95;
            std::get<1>(C) = 0;
            std::get<2>(C) = 250;
            break;
        case 8: // yellow
            std::get<0>(C) = 249;
            std::get<1>(C) = 251;
            std::get<2>(C) = 6;
            break;
        case 9: // red
            std::get<0>(C) = 255;
            std::get<1>(C) = 10;
            std::get<2>(C) = 10;
            break;
        default: // gray
            std::get<0>(C) = 100;
            std::get<1>(C) = 100;
            std::get<2>(C) = 100;
    }
    return C;
}

std::vector<ObjectDefinition> FindObjects(const std::vector<bool> &PEdges, unsigned long W, unsigned long H) {
    std::vector<Area> PosMap;
    bool PFlag = false;
    long Xs = 0;
    for (long i = 0; i < W*H; i++) {
        long Y = i / W;
        if (i && !i%W) Y--;
        long X = i - Y*W;
        if (i && PEdges[i] && !PEdges[i-1]) Xs = X;
        if (i && PEdges[i-1] && !PEdges[i]) {
            for (auto &PArea: PosMap) {
                if (std::abs(std::get<0>(PArea.back())-Xs) < 100 && std::abs(std::get<2>(PArea.back())-Y) < 50) {
                    PFlag = true;
                    PArea.emplace_back(Xs, X, Y);
                    break;
                }
            }
            if (!PFlag) {
                std::vector<PosDefinition> NArea;
                NArea.emplace_back(Xs, X, Y);
                PosMap.push_back(NArea);
            }
            PFlag = false;
        }
    }
    std::vector<ObjectDefinition> Objects;
    for (auto &PArea: PosMap) {
        long XMin = W, XMax = 0, YMin, YMax;
        for (auto &Pos: PArea) {
            if (std::get<0>(Pos) < XMin) XMin = std::get<0>(Pos);
            if (std::get<1>(Pos) > XMax) XMax = std::get<1>(Pos);
        }
        YMin = std::get<2>(PArea.front());
        YMax = std::get<2>(PArea.back());
        if (XMax-XMin > YMax-YMin) {
            auto Shift =  (XMax-XMin)/2 - (YMax-YMin)/2;
            YMin = YMin - Shift;
            YMax = YMax + Shift;
        } else if (YMax-YMin > XMax-XMin) {
            auto Shift =  (YMax-YMin)/2 - (XMax-XMin)/2;
            XMin = XMin - Shift;
            XMax = XMax + Shift;
        }
        Objects.emplace_back(XMin, XMax, YMin, YMax, -1);
    }
    return Objects;
}

void SendSignalVision(std::vector<ColorDefinition> *I1o, inn::NeuralNet *NN, int IC, int M) {
    int Mx;
    std::vector<double> X;
    auto IL = I1o[0].size();
    for (auto i = 0; i < IL; i+=8) {
        for (int m = 0; m < IC; m++) {
            if (M < 0) Mx = m;
            else Mx = M;
            X.push_back(std::get<0>(I1o[Mx][i]));
            X.push_back(std::get<1>(I1o[Mx][i]));
            X.push_back(std::get<2>(I1o[Mx][i]));
            X.push_back(std::get<0>(I1o[Mx][i+1]));
            X.push_back(std::get<1>(I1o[Mx][i+1]));
            X.push_back(std::get<2>(I1o[Mx][i+1]));
            X.push_back(std::get<0>(I1o[Mx][i+2]));
            X.push_back(std::get<1>(I1o[Mx][i+2]));
            X.push_back(std::get<2>(I1o[Mx][i+2]));
            X.push_back(std::get<0>(I1o[Mx][i+3]));
            X.push_back(std::get<1>(I1o[Mx][i+3]));
            X.push_back(std::get<2>(I1o[Mx][i+3]));
            X.push_back(std::get<0>(I1o[Mx][i+4]));
            X.push_back(std::get<1>(I1o[Mx][i+4]));
            X.push_back(std::get<2>(I1o[Mx][i+4]));
            X.push_back(std::get<0>(I1o[Mx][i+5]));
            X.push_back(std::get<1>(I1o[Mx][i+5]));
            X.push_back(std::get<2>(I1o[Mx][i+5]));
            X.push_back(std::get<0>(I1o[Mx][i+6]));
            X.push_back(std::get<1>(I1o[Mx][i+6]));
            X.push_back(std::get<2>(I1o[Mx][i+6]));
            X.push_back(std::get<0>(I1o[Mx][i+7]));
            X.push_back(std::get<1>(I1o[Mx][i+7]));
            X.push_back(std::get<2>(I1o[Mx][i+7]));

        }
        NN -> doSignalSend(X);
        X.clear();
    }
    NN -> doFinalize();
}

void GenerateOutputImage(const std::vector<ObjectDefinition> &Objects, std::vector<unsigned char> &Image, unsigned int W, unsigned int H) {
    for (auto O: Objects) {
        Color C = GetColor(std::get<4>(O));
        for (auto i = std::get<0>(O)*4; i < std::get<1>(O)*4; i += 4) {
            Image[std::get<2>(O)*W*4+i] = std::get<0>(C);
            Image[std::get<2>(O)*W*4+i+1] = std::get<1>(C);
            Image[std::get<2>(O)*W*4+i+2] = std::get<2>(C);
            Image[std::get<3>(O)*W*4+i] = std::get<0>(C);
            Image[std::get<3>(O)*W*4+i+1] = std::get<1>(C);
            Image[std::get<3>(O)*W*4+i+2] = std::get<2>(C);
        }
        for (auto i = std::get<2>(O)*4; i < std::get<3>(O)*4; i += 4) {
            Image[std::get<0>(O)*4+W*i] = std::get<0>(C);
            Image[std::get<0>(O)*4+W*i+1] = std::get<1>(C);
            Image[std::get<0>(O)*4+W*i+2] = std::get<2>(C);
            Image[std::get<1>(O)*4+W*i] = std::get<0>(C);
            Image[std::get<1>(O)*4+W*i+1] = std::get<1>(C);
            Image[std::get<1>(O)*4+W*i+2] = std::get<2>(C);
        }
    }
    lodepng::encode("../images/output.png", Image, W, H);
}

int main() {
    unsigned int IC = 10;
    auto *I1o = new std::vector<ColorDefinition>[IC];
    auto *I2o = new std::vector<ColorDefinition>;
    auto *I4R = new std::vector<ColorDefinition>;
    std::vector<bool> PEdges;
    std::vector<ObjectDefinition> Objects;
    double Time = 0;
    std::cout << "INN Vision test" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "Reading data..." << std::endl;
    unsigned int W = 0;
    unsigned int H = 0;
    std::string FN;
    std::vector<unsigned char> Image;
    for (int k = 0; k < IC; k++) {
        FN = "../images/"+std::to_string(k+1)+".png";
        lodepng::decode(Image, W, H, FN);
        for (long i = 0; i < Image.size(); i+=4) I1o[k].emplace_back(int(Image[i])/255., int(Image[i+1])/255., int(Image[i+2])/255.);
        Image.clear();
        std::cout << "Image #" << k+1 << ",\t\t\t" << W << "x" << H << "x24bit" << std::endl;
    }
    FN = "../images/F.png";
    lodepng::decode(Image, W, H, FN);
    for (long i = 0; i < Image.size(); i+=4) {
        I2o[0].emplace_back(int(Image[i])/255., int(Image[i+1])/255., int(Image[i+2])/255.);
        PEdges.push_back(!(int(Image[i]) == 13 && int(Image[i+1]) == 13 && int(Image[i+2]) == 13));
    }
    std::cout << "Image F,\t\t\t" << W << "x" << H << "x24bit" << std::endl;
    std::cout << "Data size for learning:\t\t"<< I1o[0].size()*IC*3 << " bytes" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "Detecting objects..." << std::endl;
    time_start();
    Objects = FindObjects(PEdges, W, H);
    std::cout << "Objects found on image F:\t" << Objects.size() << std::endl;
    int RS = 0;
    for (auto i = 0; i < Objects.size(); i++) {
        std::cout << "Object #" << i+1 << ": \t\t\t";
        std::cout << "X0: " << std::get<0>(Objects[i]) << ", X1: " << std::get<1>(Objects[i]);
        std::cout << ", Y0: " << std::get<2>(Objects[i]) << ", Y1: " << std::get<3>(Objects[i]) << std::endl;
        RS += (std::get<1>(Objects[i])-std::get<0>(Objects[i])) * (std::get<3>(Objects[i])-std::get<2>(Objects[i]));
    }
    std::cout << "Data size for recognition:\t"<< RS*3 << " bytes" << std::endl;
    std::vector<double> PDiff;
    Time = time_stop() / 1000.;
    std::cout << "Done:\t\t\t\t"<< Time << "s" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "Generating neural net and linking neurons..." << std::endl;
    try {
        time_start();
        auto *NN = new inn::NeuralNet();
        NN -> doCreateNewEntries(IC*4*3*2);
        for (unsigned int i = 0; i < IC; i++) {
            auto *N1 = new inn::Neuron(500, 2);
            N1->doCreateNewEntries(6);
            N1->doCreateNewSynaps(0, {200, 100}, 0, 0);
            N1->doCreateNewSynaps(1, {342, 342}, 0, 0);
            N1->doCreateNewSynaps(2, {58, 342}, 0, 0);
            N1->doCreateNewSynaps(3, {200, 400}, 0, 0);
            N1->doCreateNewSynaps(4, {58, 58}, 0, 0);
            N1->doCreateNewSynaps(5, {342, 58}, 0, 0);
            N1->doCreateNewReceptorCluster(200, 200, 10, 0);
            N1 -> setk1(1.8);
            N1 -> setk3(600.2);
            auto *N2 = new inn::Neuron(*N1);
            auto *N3 = new inn::Neuron(*N1);
            auto *N4 = new inn::Neuron(*N1);
            auto *N5 = new inn::Neuron(200, 2);
            N5->doCreateNewEntries(4);
            N5->doCreateNewSynaps(0, {50, 100}, 0, 0);
            N5->doCreateNewSynaps(1, {100, 150}, 0, 0);
            N5->doCreateNewSynaps(2, {100, 50}, 0, 0);
            N5->doCreateNewSynaps(3, {150, 100}, 0, 0);
            N5->doCreateNewReceptorCluster(100, 100, 5, 0);
            switch (i) {
                case 0:
                    N5 -> setk1(3.8);
                    N5 -> setk2(0.45);
                    break;
                case 3:
                    N5 -> setk1(3.0);
                    N5 -> setk2(0.08);
                    break;
                case 4:
                    N5 -> setk1(1.5);
                    N5 -> setk2(0.3);
                    break;
                case 5:
                    N5 -> setk1(6.0);
                    N5 -> setk2(0.09);
                    break;
                case 6:
                    N5 -> setk1(3.0);
                    N5 -> setk2(0.08);
                    break;
                default:
                    N5 -> setk1(5.5);
                    N5 -> setk2(0.2);
            }
            NN -> doAddNeuron(N1, {inn::LinkDefinitionRangeNext(inn::LINK_ENTRY2NEURON, 6)});
            NN -> doAddNeuron(N2, {inn::LinkDefinitionRangeNext(inn::LINK_ENTRY2NEURON, 6)});
            NN -> doAddNeuron(N3, {inn::LinkDefinitionRangeNext(inn::LINK_ENTRY2NEURON, 6)});
            NN -> doAddNeuron(N4, {inn::LinkDefinitionRangeNext(inn::LINK_ENTRY2NEURON, 6)});
            NN -> doAddNeuron(N5, {inn::LinkDefinition(inn::LINK_NEURON2NEURON, 5*i),
                                   inn::LinkDefinition(inn::LINK_NEURON2NEURON, 5*i+1),
                                   inn::LinkDefinition(inn::LINK_NEURON2NEURON, 5*i+2),
                                   inn::LinkDefinition(inn::LINK_NEURON2NEURON, 5*i+3)});
            NN -> doCreateNewOutput(5*i+4);
        }
        std::cout << "Preparing..." << std::endl;
        NN -> doEnableMultithreading();
        NN -> doPrepare();
        Time = time_stop() / 1000.;
        std::cout << "Done:\t\t\t\t"<< Time << "s" << std::endl;
        std::cout << "Total neurons:\t\t\t" << NN->getNeuronCount() << std::endl;
        int ECount = 0, SCount = 0, RCount = 0;
        for (unsigned int i = 0; i < NN->getNeuronCount(); i++) {
            ECount += NN -> getNeuron(i) -> getEntriesCount();
            SCount += NN -> getNeuron(i) -> getSynapsesCount();
            RCount += NN -> getNeuron(i) -> getReceptorsCount();
        }
        std::cout << "Total entries:\t\t\t" << ECount << std::endl;
        std::cout << "Total synapses:\t\t\t" << SCount << std::endl;
        std::cout << "Total receptors:\t\t" << RCount << std::endl;
        std::cout << "Total threads:\t\t\t" <<  NN->isMultithreadingEnabled()*NN->getNeuronCount()+1 << std::endl;
        std::cout << "Model ready for vision" << std::endl;
        std::cout << "=====================================================================================" << std::endl;
        std::cout << "Learning..." << std::endl;
        time_start();
        SendSignalVision(I1o, NN, IC, -1);
        Time = time_stop() / 1000.;
        std::cout << "Done:\t\t\t\t" << Time  << "s" << std::endl;
        std::cout << "Total speed:\t\t\t" << I1o[0].size()*IC*24./1024/1024/Time << " mbit/s" << std::endl;
        std::cout << "=====================================================================================" << std::endl;
        for (int i = 0; i < Objects.size(); i++) {
            NN -> doReinit();
            auto OW = std::get<1>(Objects[i]) - std::get<0>(Objects[i]);
            for (auto y = std::get<2>(Objects[i]); y < std::get<3>(Objects[i]); y++)
                for (auto x = std::get<0>(Objects[i]); x < std::get<1>(Objects[i]); x++)
                    I4R -> push_back((*I2o)[y*W+x]);
            if (OW < 120 || OW > 136) {
                for (unsigned int j = 0; j < NN->getNeuronCount(); j += 5) {
                    double Ok1 = NN->getNeuron(j)->getEntry(0)->getSynaps(0)->getk1() * inn::Neuron::System::getSynapticSensitivityValue(128, OW);
                    NN->getNeuron(j)->setk1(Ok1);
                    NN->getNeuron(j+1)->setk1(Ok1);
                    NN->getNeuron(j+2)->setk1(Ok1);
                    NN->getNeuron(j+3)->setk1(Ok1);
                }
            }
            std::cout << "Object #" << i+1 << ", Data size: " << I4R->size()*3 << " bytes" << std::endl;
            std::cout << "=====================================================================================" << std::endl;
            std::cout << "Recognizing..." << std::endl;
            time_start();
            SendSignalVision(I4R, NN, IC, 0);
            PDiff = NN -> doComparePatterns();
            auto PDMin = std::min_element(PDiff.begin(), PDiff.end());
            std::get<4>(Objects[i]) = std::distance(PDiff.begin(), PDMin);
            for (int p = 0; p < PDiff.size(); p++) std::cout << "Pattern difference #" << p+1 << ":\t\t" << PDiff[p] << std::endl;
            Time = time_stop() / 1000.;
            std::cout << "Done:\t\t\t\t" << Time << "s" << std::endl;
            std::cout << "Total speed:\t\t\t" << I4R->size()*24./1024/1024/Time << " mbit/s" << std::endl;
            std::cout << "=====================================================================================" << std::endl;
            I4R -> clear();
        }
    } catch (inn::Error &E) {
        std::cout << E.what() << std::endl;
    }
    std::cout << "Generating output and writing to \"../images/output.png\"..." << std::endl;
    time_start();
    GenerateOutputImage(Objects, Image, W, H);
    Time = time_stop() / 1000.;
    std::cout << "Done:\t\t\t\t" << Time << "s" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "Output legend:" << std::endl;
    std::cout << "Image #1:\t\t\t\033[38;5;208morange\033[1;0m" << std::endl;
    std::cout << "Image #2:\t\t\t\033[38;5;121mlight green\033[1;0m" << std::endl;
    std::cout << "Image #3:\t\t\t\033[38;5;183mlight purple\033[1;0m" << std::endl;
    std::cout << "Image #4:\t\t\t\033[38;5;223mlight yellow\033[1;0m" << std::endl;
    std::cout << "Image #5:\t\t\t\033[38;5;203mlight red\033[1;0m" << std::endl;
    std::cout << "Image #6:\t\t\t\033[38;5;51maquamarine\033[1;0m" << std::endl;
    std::cout << "Image #7:\t\t\t\033[38;5;82mgreen\033[1;0m" << std::endl;
    std::cout << "Image #8:\t\t\t\033[38;5;164mpurple\033[1;0m" << std::endl;
    std::cout << "Image #9:\t\t\t\033[38;5;226myellow\033[1;0m" << std::endl;
    std::cout << "Image #10:\t\t\t\033[38;5;197mred\033[1;0m" << std::endl;
    std::cout << "Unknown:\t\t\t\033[38;5;249mgray\033[1;0m" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    return 0;
}
