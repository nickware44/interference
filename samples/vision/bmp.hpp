//
// Created by nickware on 30.01.2023.
//

#ifndef INTERFERENCE_BMP_HPP
#define INTERFERENCE_BMP_HPP

#include <iostream>
#include <vector>
#include <fstream>

typedef std::vector<std::array<uint8_t, 3>> BMPImage;

inline BMPImage doReadBMP(const std::string& filename) {
    std::ifstream fbmp(filename);
    unsigned char info[54];
    std::vector<std::array<uint8_t, 3>> dbmp;

    if(!fbmp.is_open()) return {};
    fbmp.read(reinterpret_cast<char*>(info), 54);

    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    int row_padded = (width*3 + 3) & (~3);
    auto* data = new uint8_t[row_padded];

    for(int i = 0; i < height; i++) {
        fbmp.read((char*)data, row_padded);
        for(int j = 0; j < width*3; j += 3) {
            dbmp.push_back({data[j+2], data[j+1], data[j]});
        }
    }

    fbmp.close();
    delete [] data;
    return dbmp;
}

#endif //INTERFERENCE_BMP_HPP
