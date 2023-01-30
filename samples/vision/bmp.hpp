//
// Created by nickware on 30.01.2023.
//

#ifndef INTERFERENCE_BMP_HPP
#define INTERFERENCE_BMP_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

class BitMap {

private:
    unsigned char m_bmpFileHeader[14];
    unsigned int m_pixelArrayOffset;
    unsigned char m_bmpInfoHeader[40];

    int m_height;
    int m_width;
    int m_bitsPerPixel;

    int m_rowSize;
    int m_pixelArraySize;

    unsigned char* m_pixelData;

    char * m_copyname;
    const char * m_filename;
public:
    BitMap(const char * filename);
    ~BitMap();

    std::vector<unsigned int> getPixel(int i,int j);

    void makeCopy(char * filename);
    void writePixel(int i,int j, int R, int G, int B);

    void swapPixel(int i, int j, int i2, int j2);

    void dispPixelData();

    int width() {return m_width;}
    int height() {return m_height;}

    int vd(int i, int j);
    int hd(int i, int j);

    bool isSorted();
};

BitMap::BitMap( const char * filename) {

    using namespace std;

    m_filename = filename;

    ifstream inf(filename);
    if(!inf) {
        cerr<<"Unable to open file: "<<filename<<"\n";
    }



    //unsigned char m_bmpFileHeader[14];
    unsigned char a;
    for(int i =0;i<14;i++) {
        inf>>hex>>a;
        m_bmpFileHeader[i] = a;
    }
    if(m_bmpFileHeader[0]!='B' || m_bmpFileHeader[1]!='M') {
        cerr<<"Your info header might be different!\nIt should start with 'BM'.\n";
    }

    /*
        THE FOLLOWING LINE ONLY WORKS IF THE OFFSET IS 1 BYTE!!!!! (it can be 4 bytes max)
        That should be fixed now.
        old line was
        m_pixelArrayOffset = m_bmpFileHeader[10];
    */
    unsigned int * array_offset_ptr = (unsigned int *)(m_bmpFileHeader + 10);
    m_pixelArrayOffset = *array_offset_ptr;


    if( m_bmpFileHeader[11] != 0 || m_bmpFileHeader[12] !=0 || m_bmpFileHeader[13] !=0 ) {
        std::cerr<< "You probably need to fix something. bmp.h("<<__LINE__<<")\n";
    }



    //unsigned char m_bmpInfoHeader[40];
    for(int i=0;i<40;i++) {
        inf>>hex>>a;
        m_bmpInfoHeader[i]=a;
    }

    int * width_ptr = (int*)(m_bmpInfoHeader+4);
    int * height_ptr = (int*)(m_bmpInfoHeader+8);

    m_width = *width_ptr;
    m_height = *height_ptr;

    printf("W: %i, H: %i", m_width, m_height);

    m_bitsPerPixel = m_bmpInfoHeader[14];
    if(m_bitsPerPixel!=24) {
        cerr<<"This program is for 24bpp files. Your bmp is not that\n";
    }
    int compressionMethod = m_bmpInfoHeader[16];
    if(compressionMethod!=0) {
        cerr<<"There's some compression stuff going on that we might not be able to deal with.\n";
        cerr<<"Comment out offending lines to continue anyways. bpm.h line: "<<__LINE__<<"\n";
    }


    m_rowSize = int( floor( (m_bitsPerPixel*m_width + 31.)/32 ) ) *4;
    m_pixelArraySize = m_rowSize* abs(m_height);

    m_pixelData = new unsigned char [m_pixelArraySize];

    inf.seekg(m_pixelArrayOffset,ios::beg);
    for(int i=0;i<m_pixelArraySize;i++) {
        inf>>hex>>a;
        m_pixelData[i]=a;
    }



}

BitMap::~BitMap() {
    delete[] m_pixelData;
}

void BitMap::dispPixelData() {
    for(int i=0;i<m_pixelArraySize;i++) {
        std::cout<<(unsigned int)m_pixelData[i]<<" ";
    }
    std::cout<<"\n";
}

// output is in rgb order.
std::vector<unsigned int> BitMap::getPixel(int x, int y) {
    if(x<m_width && y<m_height) {
        std::vector<unsigned int> v;
        v.push_back(0);
        v.push_back(0);
        v.push_back(0);

        y = m_height -1- y; //to flip things
        //std::cout<<"y: "<<y<<" x: "<<x<<"\n";
        v[0] = (unsigned int) ( m_pixelData[ m_rowSize*y+3*x+2 ] ); //red
        v[1] = (unsigned int) ( m_pixelData[ m_rowSize*y+3*x+1 ] ); //greed
        v[2] = (unsigned int) ( m_pixelData[ m_rowSize*y+3*x+0 ] ); //blue


        return v;
    }
    else {std::cerr<<"BAD INDEX\n";std::cerr<<"X: "<<x<<" Y: "<<y<<"\n";}
}

void BitMap::makeCopy(char * filename) {
    std::ofstream copyfile(filename);
    std::ifstream infile(m_filename);
    m_copyname = filename;

    unsigned char c;
    while(infile) {
        infile>>c;
        copyfile<<c;
    }
}

// changes the file
void BitMap::writePixel(int x,int y, int R, int G, int B) {
    std::fstream file(m_filename);
    y = m_height -1- y; // to flip things.
    int blueOffset = m_pixelArrayOffset+m_rowSize*y+3*x+0;

    // writes to the file
    file.seekg(blueOffset,std::ios::beg);
    file<< (unsigned char)B;
    file.seekg(blueOffset+1,std::ios::beg);
    file<< (unsigned char)G;
    file.seekg(blueOffset+2,std::ios::beg);
    file<< (unsigned char)R;

    // edits data in pixelData array
    m_pixelData[m_rowSize*y+3*x+2] = (unsigned char)R;
    m_pixelData[m_rowSize*y+3*x+1] = (unsigned char)G;
    m_pixelData[m_rowSize*y+3*x+0] = (unsigned char)B;
}

// changes the file
void BitMap::swapPixel(int i, int j, int i2, int j2) {
    std::vector<unsigned int> p1 = (*this).getPixel(i,j);

    std::vector<unsigned int> p2 = (*this).getPixel(i2,j2);

    (*this).writePixel(i,j,p2[0],p2[1],p2[2]);
    (*this).writePixel(i2,j2,p1[0],p1[1],p1[2]);

}

#endif //INTERFERENCE_BMP_HPP
