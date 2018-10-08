//
// Created by warchlak on 07.10.18.
//
#include "ImageData.h"

template<typename T>
ImageData<T>::ImageData(T *data, int rows, int columns, int channels, int type)
        : numOfRows(rows), numOfColumns(columns), numOfChannels(channels), cvType(type),
          dataStart(data), dataEnd(dataStart + rows * columns * channels)
{
}

template<typename T>
T *ImageData<T>::getData()
{
    return dataStart;
}

//template<typename T>
//template<const int dimSize, typename H>
//void ImageData<T>::applyKernel(const TransformationMatrix<dimSize, H> &kernel)
//{
//    T *data = dataStart;
//
//    const int distanceToEdges = dimSize % 3;
//
//    const int rowStartIndex = distanceToEdges;
//    const int rowEndIndex = numOfColumns - distanceToEdges - 1;
//
//    const int columnStartIndex = distanceToEdges;
//    const int columnEndIndex = numOfRows - distanceToEdges - 1;
//
//    H coefficientsSum = kernel.getCoefficientsSum();
//
//    T *centerPixel;
//    T *topLeftPixel;
//    H localCoefficient;
//
//    for (int i = rowStartIndex; i < rowEndIndex; i++)
//    {
//        for (int j = columnStartIndex; j < columnEndIndex; j++)
//        {
//
//            centerPixel = getPixelAt(i, j);
//            topLeftPixel = getTopLeftPixel(centerPixel, dimSize);
//            localCoefficient = 0;
//
//            for (int rowOffset = 0; rowOffset < dimSize; rowOffset++)
//            {
//                for (int columnOffset = 0; columnOffset < dimSize; columnOffset++)
//                {
//                    localCoefficient += *(topLeftPixel + columnOffset + rowOffset * numOfColumns)
//                                        * kernel.valueAt(rowOffset, columnOffset);
//                }
//            }
//
//            *centerPixel = (T) (localCoefficient / coefficientsSum);
//        }
//    }
//}

template<typename T>
T *ImageData<T>::getPixelAt(int row, int column)
{
    return (dataStart + column + row * numOfColumns * numOfChannels);
}

template<typename T>
T *ImageData<T>::getTopLeftPixel(T *centerPixel, int offset)
{
    return (centerPixel + offset + offset * numOfColumns * numOfChannels);
}


