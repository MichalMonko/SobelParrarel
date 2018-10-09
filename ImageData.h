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

    T *getPixelRelativeTo(T *centerPixel, int offsetX, int offsetY);

public:
    T *getData();

//    template<const int dimSize, typename H>
//    void applyKernel(const TransformationMatrix<dimSize, H> &kernel);

    template<const int dimSize, typename H>
    void applyConvolutionKernels(TransformationMatrix<dimSize, H> &kernelX, TransformationMatrix<dimSize, H> &kernelY,
                                 const unsigned char THRESHOLD_LEVEL = 0)
    {
        T *imageCopyBlock = (T *) malloc(numOfColumns * numOfRows * numOfChannels * sizeof(T));
        T *convolutedImage = imageCopyBlock;
        memcpy(imageCopyBlock, dataStart, numOfColumns * numOfRows * numOfChannels * sizeof(T));

        const int distanceToEdges = dimSize / 2;

        const int rowStartIndex = distanceToEdges;
        const int lastRowIndex = numOfRows - distanceToEdges;

        const int columnStartIndex = distanceToEdges;
        const int lastColumnIndex = numOfColumns - distanceToEdges;

        T *topLeftPixel;
        H localCoefficientX;
        H localCoefficientY;

        for (int i = rowStartIndex; i < lastRowIndex; i++)
        {
            for (int j = columnStartIndex; j < lastColumnIndex; j++)
            {
                topLeftPixel = getPixelAt(i - distanceToEdges, j - distanceToEdges);
                localCoefficientX = 0;
                localCoefficientY = 0;

                for (int rowOffset = 0; rowOffset < dimSize; rowOffset++)
                {
                    for (int columnOffset = 0; columnOffset < dimSize; columnOffset++)
                    {
                        H kernelValueForPixelX = kernelX.valueAt(rowOffset, columnOffset);
                        H kernelValueForPixelY = kernelY.valueAt(rowOffset, columnOffset);

                        T *currentPixel = getPixelRelativeTo(topLeftPixel, columnOffset, rowOffset);

                        localCoefficientX += (*currentPixel) * kernelValueForPixelX;
                        localCoefficientY += (*currentPixel) * kernelValueForPixelY;
                    }
                }

                T pixelValue = (T) abs(localCoefficientX) + abs(localCoefficientY);
                T *convolutedImagePixel = convolutedImage + j + i * numOfColumns * numOfChannels;

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
