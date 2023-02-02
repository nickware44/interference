//
// Created by nickware on 30.01.2023.
//

#ifndef INTERFERENCE_BMP_HPP
#define INTERFERENCE_BMP_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>

typedef std::vector<std::array<uint8_t, 3>> BMPImage;

inline BMPImage doReadBMP(const std::string& filename) {
    std::ifstream fbmp(filename, std::ios::binary);
    unsigned char info[54];
    std::vector<std::array<uint8_t, 3>> dbmp;

    if(!fbmp.is_open()) return {};
    fbmp.read(reinterpret_cast<char*>(info), 54);

    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    auto datasize = width * height * 3;
    auto* data = new uint8_t[datasize];
    memset(data, 0x0, datasize);
    fbmp.read(reinterpret_cast<char*>(data), datasize);
    for (int i = 0; i < datasize; i+=3) {
        dbmp.push_back({data[i+2], data[i+1], data[i]});
    }

    fbmp.close();
    delete [] data;
    return dbmp;
}

#endif //INTERFERENCE_BMP_HPP
