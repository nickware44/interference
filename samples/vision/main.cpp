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

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

uint32_t make_stride_aligned(uint32_t align_stride, uint32_t row_stride) {
    uint32_t new_stride = row_stride;
    while (new_stride % align_stride != 0) {
        new_stride++;
    }
    return new_stride;
}

std::vector<uint8_t> doReadBMP(const std::string& filename) {
    int pwidth = 128, pheight = 128, pbits = 24;
    std::vector<uint8_t> data;

    std::ifstream inp{filename, std::ios_base::binary};
    uint32_t row_stride = pwidth * pbits / 8;
    uint32_t new_stride = make_stride_aligned(4, row_stride);
    std::vector<uint8_t> padding_row(new_stride - row_stride);
    for (int y = 0; y < pheight; ++y) {
        inp.read((char*)(data.data() + row_stride * y), row_stride);
        inp.read((char*)padding_row.data(), padding_row.size());
    }
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

    doReadBMP("../samples/vision/images/1.bmp");
}
