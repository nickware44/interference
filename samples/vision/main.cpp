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
#include <fstream>
#include "inn/system.h"
#include "inn/neuralnet.h"
#include "bmp.hpp"

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

unsigned char* ReadBMP(char* filename)
{
    int i;
    FILE* f = fopen(filename, "rb");

    if(f == NULL)
        throw "Argument Exception";

    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    std::cout << std::endl;
    std::cout << "  Name: " << filename << std::endl;
    std::cout << " Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;

    int row_padded = (width*3 + 3) & (~3);
    unsigned char* data = new unsigned char[row_padded];
    unsigned char tmp;
    int x = 0;
    for(int i = 0; i < height; i++)
    {
        fread(data, sizeof(unsigned char), row_padded, f);
        x = 0;
        for(int j = 0; j < width*3; j += 3)
        {
            x++;
            // Convert (B, G, R) to (R, G, B)
            tmp = data[j];
            data[j] = data[j+2];
            data[j+2] = tmp;

            std::cout << i+1 << "x" << x <<  " R: "<< (int)data[j] << " G: " << (int)data[j+1]<< " B: " << (int)data[j+2]<< std::endl;
        }
    }

    fclose(f);
    return data;
}

int main() {
    inn::System::setVerbosityLevel(1);

    std::ifstream structure("../samples/vision/structure.json");
    auto NN = new inn::NeuralNet();
    NN -> setStructure(structure);

    std::cout << "Model name  : " << NN->getName() << std::endl;
    std::cout << "Model desc  : " << NN->getDescription() << std::endl;
    std::cout << "Model ver   : " << NN->getVersion() << std::endl;
    std::cout << std::endl;

    ReadBMP("../samples/vision/images/1.bmp");
}
