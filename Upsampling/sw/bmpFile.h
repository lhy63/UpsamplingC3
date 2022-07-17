#pragma once
#include<fstream>
#include<iostream>
#include"RGBTriple.h"
#define BMPTYPE 19778
 
using namespace std;
 
#pragma pack(2)
struct BmpFileHeader {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
};
 
struct BmpFileInFoHeader {
    unsigned int biSize;
    int biWidth = 0, biHeight = 0;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression, biSizeImages;
    int biXPelsPerMeter, biYPelsPerMeter;
    unsigned int biClrUsed, biClrImportant;
};
 
 
 
class bmp {
private:
    int offset;
    RGBTriple* surface;
    BmpFileHeader fileHeader;
    BmpFileInFoHeader fileInFoHeader;
 
public:
    bmp(void);
    bmp(int Offset, RGBTriple* Surface, BmpFileHeader FileHeader, BmpFileInFoHeader FileInFoHeader);
    void readPic(std::string fileName);
    void writePic(std::string outFileName);
    BmpFileHeader getBmpFileHeader(void);
    BmpFileInFoHeader getBmpFileInFoHeader(void);
    RGBTriple* getRGBTriple(void);
};
 
 