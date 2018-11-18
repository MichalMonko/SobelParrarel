//
// Created by warchlak on 07.10.18.
//
#include "include/ImageDataClass.h"

ImageDataClass::ImageDataClass(unsigned char *data, int rows, int columns, int channels)
        : numOfRows(rows), numOfColumns(columns), numOfChannels(channels)
{
    dataStart = data;
}

u_char *ImageDataClass::getPixelAt(int row, int column)
{
    return (dataStart + column + row * numOfColumns * numOfChannels);
}

u_char ImageDataClass::getPixelValueRelativeTo(int row, int col, int rowOffset, int colOffset, BORDER_TYPE border_type)
{
    int rowIndex = row + rowOffset;
    int colIndex = col + colOffset;

    if (border_type == NN_CLONE)
    {
        if (rowIndex >= numOfRows)
        {
            rowIndex = numOfRows - 1;
        } else if (rowIndex < 0)
        {
            rowIndex = 0;
        }
        if (colIndex >= numOfColumns)
        {
            colIndex = numOfRows - 1;
        } else if (colIndex < 0)
        {
            colIndex = 0;
        }
    } else if (border_type == ZERO_FILL)
    {
        if (rowIndex >= numOfRows || colIndex >= numOfColumns || rowIndex < 0 || colIndex < 0)
        {
            return 0;
        }
    }

    u_char *pixel = getPixelAt(rowIndex, colIndex);
    return (*pixel);
}

unsigned char const *ImageDataClass::getData()
{
    return dataStart;
}

ImageDataClass::~ImageDataClass()
{
    if (dataStart != nullptr)
    {
        free(dataStart);
    }
}


