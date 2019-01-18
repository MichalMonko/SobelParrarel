#include <cstdlib>
#include <iostream>
#include "BorderType.h"

//Klasa przechcowująca obraz w postaci wskaźnika do jego początku oraz definiująca potrzebne operacje
class ImageDataClass
{
public:
    ImageDataClass(unsigned char *data, int rows, int columns, int channels);
    ~ImageDataClass();

    const int numOfRows;
    const int numOfColumns;
    const int numOfChannels;

    unsigned char getPixelValueRelativeTo(int row, int col, int rowOffset, int colOffset, BORDER_TYPE border_type);
    unsigned char const * getData();
    unsigned char *dataStart;

private:
    unsigned char *getPixelAt(int row, int column);
};
