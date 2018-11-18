//
// Created by warchlak on 16.11.18.
//

#include <cmath>
#include "include/ImageTransformation.h"

u_char THRESHOLD = 100;

u_char max(u_char value, u_char threshold);

unsigned char median_filter(unsigned char pixel_area[], int dimSize)
{
    insertion_sort(pixel_area, dimSize);

    int middle_index = dimSize * dimSize / 2;

    return pixel_area[middle_index];
}

unsigned char insertion_sort(unsigned char pixel_area[], int dimSize)
{
    unsigned char key;
    int j;

    int size = dimSize * dimSize;
    int middle_value = size / 2;

    for (int i = 1; i < size; i++)
    {
        key = pixel_area[i];
        j = i - 1;

        while (pixel_area[j] > key && j >= 0)
        {
            pixel_area[j + 1] = pixel_area[j];
            j--;
        }
        pixel_area[j + 1] = key;
    }

    return pixel_area[middle_value];
}

u_char sum_pixel_values(unsigned char *pixel_area, int dimSize)
{
    u_char sum = 0;
    for (int i = 0; i < (dimSize * dimSize); i++)
    {
        sum += pixel_area[i];
    }
    return sum;
}

u_char sum_pixel_values(double *pixel_area, int dimSize)
{
    double sum = 0;
    for (int i = 0; i < (dimSize * dimSize); i++)
    {
        sum += pixel_area[i];
    }
    return (u_char) sum;
}

u_char *multiply_and_sqrt_each_pixel(ImageDataClass *img1, ImageDataClass *img2)
{
    int numOfRows = img1->numOfRows;
    int numOfColumns = img1->numOfColumns;

    if (numOfRows != img2->numOfRows || numOfColumns != img2->numOfColumns)
    {
        return nullptr;
    }

    const u_char *image_matrix_1 = img1->getData();
    const u_char *image_matrix_2 = img1->getData();

    auto *new_image = (u_char *) malloc(sizeof(u_char) * numOfColumns * numOfRows);

    u_char pixelValue1;
    u_char pixelValue2;
    u_char pixelValueResult;

    for (int row = 0; row < numOfRows; row++)
    {
        for (int col = 0; col < numOfColumns; col++)
        {
            pixelValue1 = image_matrix_1[numOfColumns * row + col];
            pixelValue2 = image_matrix_2[numOfColumns * row + col];
            pixelValueResult = (u_char) sqrt(pixelValue1 * pixelValue1 + pixelValue2 * pixelValue2);
            pixelValueResult = (pixelValueResult > THRESHOLD) ? (u_char) 255 : (u_char) 0;
            new_image[numOfColumns * row + col] = pixelValueResult;
        }
    }

    return new_image;
}

u_char sum_pixel_values_absolute(double *pixel_area, int dimSize)
{
    double sum = 0;
    for (int i = 0; i < (dimSize * dimSize); i++)
    {
        sum += pixel_area[i];
    }
    return (u_char) fabs(sum);
}

u_char sum_pixel_values_with_threshold(double *pixel_area, int dimSize)
{
    double sum = 0;
    for (int i = 0; i < (dimSize * dimSize); i++)
    {
        sum += pixel_area[i];
    }
    auto value = (u_char) fabs(sum);
    return max(value, THRESHOLD);

}

u_char max(u_char value1, u_char value2)
{
    return (value1 > value2) ? value1 : value2;
}

void set_threshold(u_char threshold)
{
    THRESHOLD = threshold;
}


























