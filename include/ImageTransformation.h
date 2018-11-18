//
// Created by warchlak on 16.11.18.
//

#include "BorderType.h"
#include "ImageDataClass.h"
#include "TransformationMatrix.h"

#ifndef SOBEL_IMAGETRANSFORMATION_H
#define SOBEL_IMAGETRANSFORMATION_H


u_char *multiply_and_sqrt_each_pixel(ImageDataClass *img1, ImageDataClass *img2);

u_char median_filter(unsigned char pixel_area[], int dimSize);

u_char sum_pixel_values(unsigned char pixel_area[], int dimSize);

u_char sum_pixel_values(double pixel_area[], int dimSize);

u_char sum_pixel_values_absolute(double pixel_area[], int dimSize);

u_char sum_pixel_values_with_threshold(double pixel_area[], int dimSize);

void set_threshold(u_char threshold);

u_char insertion_sort(unsigned char pixel_area[], int dimSize);

template<typename T>
unsigned char *convolve(ImageDataClass *imageData, int dimSize, unsigned char (*transformation)(T *, int),
                        TransformationMatrix<T> *kernel, BORDER_TYPE border_type)
{
    int numOfRows = imageData->numOfRows;
    int numOfColumns = imageData->numOfColumns;

    T pixel_area[dimSize * dimSize];

    auto *processedImage = (unsigned char *) malloc(numOfColumns * numOfRows * sizeof(unsigned char));

    int topLeftPixelRow, topLeftPixelColumn, offset_to_border;
    offset_to_border = dimSize / 2;

    for (int row = 0; row < numOfRows; row++)
    {
        for (int col = 0; col < numOfColumns; col++)
        {
            topLeftPixelRow = row - offset_to_border;
            topLeftPixelColumn = col - offset_to_border;
            for (int rowOffset = 0; rowOffset < dimSize; rowOffset++)
            {
                for (int columnOffset = 0; columnOffset < dimSize; columnOffset++)
                {
                    pixel_area[rowOffset * dimSize + columnOffset] =
                            (T) imageData->getPixelValueRelativeTo(topLeftPixelRow, topLeftPixelColumn, rowOffset,
                                                                   columnOffset, border_type);
                }
            }

            if (kernel != nullptr)
            {
                kernel->apply_kernel(pixel_area, dimSize);
            }
            if (transformation != nullptr)
            {
                processedImage[row * numOfColumns + col] = transformation(pixel_area, dimSize);
            }
        }
    }
    return processedImage;
}


#endif //SOBEL_IMAGETRANSFORMATION_H
