//
// Created by warchlak on 07.10.18.
//

#ifndef SOBEL_IMAGEDATA_H
#define SOBEL_IMAGEDATA_H

#include "TransformationMatrix.h"

template<typename T>
class ImageData
{
public:
    ImageData(T *data, int rows, int columns, int channels, int type);

    const int numOfRows;
    const int numOfColumns;
    const int cvType;
    const int numOfChannels;

private:
    T *const dataStart;
    T *const dataEnd;

    T *getPixelAt(int row, int column);

    T *getTopLeftPixel(T *centerPixel, int offset);

public:
    T *getData();

//    template<const int dimSize, typename H>
//    void applyKernel(const TransformationMatrix<dimSize, H> &kernel);

    template<const int dimSize, typename H>
    void applyKernel( TransformationMatrix<dimSize, H> &kernel)
    {
        T *data = dataStart;

        const int distanceToEdges = dimSize % 3;

        const int rowStartIndex = distanceToEdges;
        const int rowEndIndex = numOfColumns - distanceToEdges - 1;

        const int columnStartIndex = distanceToEdges;
        const int columnEndIndex = numOfRows - distanceToEdges - 1;

        H coefficientsSum = kernel.getCoefficientsSum();

        T *centerPixel;
        T *topLeftPixel;
        T localCoefficient;

        for (int i = rowStartIndex; i < rowEndIndex; i++)
        {
            for (int j = columnStartIndex; j < columnEndIndex; j++)
            {

                centerPixel = getPixelAt(i, j);
                topLeftPixel = getTopLeftPixel(centerPixel, dimSize);
                localCoefficient = 0;

                for (int rowOffset = 0; rowOffset < dimSize; rowOffset++)
                {
                    for (int columnOffset = 0; columnOffset < dimSize; columnOffset++)
                    {
                        localCoefficient += *(topLeftPixel + columnOffset + rowOffset * numOfColumns)
                                            * kernel.valueAt(rowOffset, columnOffset);
                    }
                }

                *centerPixel = (T) (localCoefficient / coefficientsSum);
            }
        }
    }
};

template
class ImageData<unsigned char>;

#endif //SOBEL_IMAGEDATA_H
