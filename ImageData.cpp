//
// Created by warchlak on 07.10.18.
//
#include "ImageData.h"

template<typename T>
ImageData<T>::ImageData(T *data, int rows, int columns, int channels, int type)
        : numOfRows(rows), numOfColumns(columns), numOfChannels(channels), cvType(type),
          dataEnd(dataStart + rows * columns * channels)
{
    dataStart = data;
}

template<typename T>
T *ImageData<T>::getData()
{
    return dataStart;
}

template<typename T>
T *ImageData<T>::getPixelAt(int row, int column)
{
    return (dataStart + column + row * numOfColumns * numOfChannels);
}

template<typename T>
T *ImageData<T>::getPixelRelativeTo(T *centerPixel, int offsetX, int offsetY)
{
    return (centerPixel + offsetX + offsetY * numOfColumns * numOfChannels);
}


