//
// Created by warchlak on 07.10.18.
//

#ifndef SOBEL_GAUSSIANKERNEL_H
#define SOBEL_GAUSSIANKERNEL_H

#include "Kernel.h"

template<const int dimSize, typename T>
class GaussianKernel : public Kernel<dimSize, T>
{
public:

    explicit GaussianKernel(const TransformationMatrix<dimSize, T> &matrix)
            : Kernel<dimSize, T>(matrix)
    {
        image = nullptr;
        this->matrix = matrix;
    }

private:
    unsigned char *image;

protected:
    void operator*(ImageMetadata &imageMetadata) override
    {
        image = imageMetadata.getData();

        const int distanceToEdges = dimSize % 3;

        const int rowStartIndex = distanceToEdges;
        const int rowEndIndex = imageMetadata.numOfColumns - distanceToEdges - 1;

        const int columnStartIndex = distanceToEdges;
        const int columnEndIndex = imageMetadata.numOfRows - distanceToEdges - 1;

        const int rowStepSize = imageMetadata.numOfChannels * imageMetadata.numOfColumns;
        const int columnStepSize = imageMetadata.numOfChannels;

        T pixelValueR;
        T pixelValueG;
        T pixelValueB;

        T coefficientsSum = 0;
        for (int i = 0; i < dimSize * 2; i++)
        {
            coefficientsSum += this->matrix.coefficients[i];
        }


        for (int i = rowStartIndex; i < rowEndIndex; i++)
        {
            for (int j = columnStartIndex; j < columnEndIndex; j++)
            {
                unsigned char *topLeftValue = image + (i - distanceToEdges) * rowStepSize
                                              + (j - distanceToEdges) * columnStepSize;
                for (int k = 0; k < dimSize; k++)
                {
                    for (int n = 0; n < dimSize; n++)
                    {
                        const T coefficient = this->matrix.coefficients[n * dimSize + k * dimSize];
                        pixelValueB = (T) *(topLeftValue + n * columnStepSize + k * rowStepSize) * coefficient;
                        pixelValueG = (T) *(topLeftValue + 1 + n * columnStepSize + k * rowStepSize) * coefficient;
                        pixelValueR = (T) *(topLeftValue + 2 + n * columnStepSize + k * rowStepSize) * coefficient;
                    }
                }

            }
        }
    }

};

#endif //SOBEL_GAUSSIANKERNEL_H
