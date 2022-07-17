#include"bmpFile.h"
 
bmp::bmp(int Offset, RGBTriple* Surface, BmpFileHeader FileHeader, BmpFileInFoHeader FileInFoHeader){
    offset = Offset;
    surface = Surface;
    fileHeader = FileHeader;
    fileInFoHeader = FileInFoHeader;
}
bmp::bmp(){
        //无参构造函数
    }
 
BmpFileHeader bmp::getBmpFileHeader(void){
   return fileHeader;
}

BmpFileInFoHeader bmp::getBmpFileInFoHeader(void){
   return fileInFoHeader;
}

RGBTriple* bmp::getRGBTriple(void){
   return surface;
}
 
void bmp::readPic(std::string fileName) {
 
    ifstream ifs;
    ifs.open(fileName, ios::in|ios::binary);
 
    if (!ifs.is_open()) {
        cout << "Can't open the file." << endl;
        return;
    }
 
    ifs.read((char*)&fileHeader, sizeof(BmpFileHeader));
 
    if (fileHeader.bfType != BMPTYPE) {
        cout << "type error!" << endl;
        return;
    }
 
    ifs.read((char*)&fileInFoHeader, sizeof(BmpFileInFoHeader));
 
    if (fileInFoHeader.biBitCount != 24) {
        cout << "invalid!" << endl;   
        return;
    }
 
    offset = (fileInFoHeader.biWidth * 3) % 4;
    if (offset != 0) {
        offset = 4 - offset;
    }
    
 
    surface = new RGBTriple[fileInFoHeader.biHeight * fileInFoHeader.biWidth];
 
    for (int i = fileInFoHeader.biHeight-1;i >=0;i--) {
        for (int j = 0;j < fileInFoHeader.biWidth;j++) {
 
            ifs.read((char*)(surface+(fileInFoHeader.biWidth * i + j)), sizeof(RGBTriple));   
 
        }
 
        if (offset != 0) {
            char ign;
            for (int k = 0;k < offset;k++) {
                ifs.read(&ign,sizeof(char));
            }
        }
 
    }
 
    ifs.close();
}
void bmp::writePic(std::string outFileName) {
 
    //create a new bmp
    ofstream ofs;
    ofs.open(outFileName, ios::out|ios::binary);
 
    ofs.write((char*)&fileHeader, sizeof(BmpFileHeader));
    ofs.write((char*)&fileInFoHeader, sizeof(BmpFileInFoHeader));
 
    //rewrite
    for (int i = fileInFoHeader.biHeight - 1;i >= 0;i--) {
        for (int j = 0;j < fileInFoHeader.biWidth;j++) {
            ofs.write((char*)(surface + (i*fileInFoHeader.biWidth+j)), sizeof(RGBTriple));
        }
 
        if (offset != 0) {
            char ign=0;
            for (int k = 0;k < offset;k++) {
                ofs.write(&ign, sizeof(char));
            }
        }
 
    }
 
    delete[] surface;
    ofs.close();
}
