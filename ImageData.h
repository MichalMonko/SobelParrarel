//
// Created by warchlak on 07.10.18.
//

#ifndef SOBEL_IMAGEDATA_H
#define SOBEL_IMAGEDATA_H

#include <cstdlib>
#include <iostream>
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
    T *dataStart;
    T *dataEnd;

    T *getPixelAt(int row, int column);

    T getPixelValueRelativeTo(int row, int col, int rowOffset, int colOffset);

public:
    T *getData();

//    template<const int dimSize, typename H>
//    void applyKernel(const TransformationMatrix<dimSize, H> &kernel);

    template<const int dimSize, typename H>
    void applyConvolutionKernels(TransformationMatrix<dimSize, H> &kernelX, TransformationMatrix<dimSize, H> &kernelY,
                                 const unsigned char THRESHOLD_LEVEL = 0)
    {

        T *convolutedImage = (T *) malloc(numOfColumns * numOfRows * numOfChannels * sizeof(T));
        memcpy(convolutedImage, dataStart, numOfColumns * numOfRows * numOfChannels * sizeof(T));

        const int lastRowIndex = numOfRows - 1;

        const int lastColumnIndex = numOfColumns - 1;

        T *topLeftPixel;
        H localCoefficientX;
        H localCoefficientY;

        for (int row = 0; row < lastRowIndex; row++)
        {
            for (int col = 0; col < lastColumnIndex; col++)
            {
                localCoefficientX = 0;
                localCoefficientY = 0;

                for (int rowOffset = 0; rowOffset < dimSize; rowOffset++)
                {
                    for (int columnOffset = 0; columnOffset < dimSize; columnOffset++)
                    {
                        H kernelValueForPixelX = kernelX.valueAt(rowOffset, columnOffset);
                        H kernelValueForPixelY = kernelY.valueAt(rowOffset, columnOffset);

                        T currentPixelValue = getPixelValueRelativeTo(row, col, rowOffset, columnOffset);

                        localCoefficientX += currentPixelValue * kernelValueForPixelX;
                        localCoefficientY += currentPixelValue * kernelValueForPixelY;
                    }
                }

                T pixelValue = (T) abs(localCoefficientX) + abs(localCoefficientY);
                T *convolutedImagePixel = convolutedImage + col + row * numOfColumns * numOfChannels;

                *convolutedImagePixel = MAX(pixelValue, THRESHOLD_LEVEL);
            }
        }

        free(dataStart);
        dataStart = convolutedImage;
    }
};

template
class ImageData<unsigned char>;

#endif //SOBEL_IMAGEDATA_H
