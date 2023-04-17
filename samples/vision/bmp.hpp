//
// Created by nickware on 30.01.2023.
//

#ifndef INTERFERENCE_BMP_HPP
#define INTERFERENCE_BMP_HPP

#include <cmath>
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

inline std::vector<float> RGB2CMYK(float r, float g, float b) {
    auto rgb = std::vector<float>({r, g, b});
    auto max = std::max_element(rgb.begin(), rgb.end());

    auto k = 1 - rgb[std::distance(rgb.begin(), max)];
    auto c = (1-rgb[0]-k) / (1-k);
    auto m = (1-rgb[1]-k) / (1-k);
    auto y = (1-rgb[2]-k) / (1-k);

    return {c, m, y, k};
}

inline std::vector<float> RGB2HSI(float r, float g, float b) {
    auto rgb = std::vector<float>({r, g, b});
    auto min = std::min_element(rgb.begin(), rgb.end());

    float h = 0;
    float s = 0;
    float i = (r + g + b) / 3;

    float rn = r / (r + g + b);
    float gn = g / (r + g + b);
    float bn = b / (r + g + b);

    if (i > 0) s = 1 - (rgb[std::distance(rgb.begin(), min)] / i);
    if (s < 1e-8) s = 0;
    if (s > 0) {
        h = acos((0.5 * ((rn - gn) + (rn - bn))) / (std::sqrt((rn - gn) * (rn - gn) + (rn - bn) * (gn - bn))));
        if (b > g) h = 2 * M_PI - h;
    }
    return {h, s, i};
}

#endif //INTERFERENCE_BMP_HPP
