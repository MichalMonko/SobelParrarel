//
// Created by warchlak on 08.10.18.
//
#include <stdexcept>
#include <assert.h>
#include <cmath>
#include "include/TransformationMatrix.h"

void multiply_each(unsigned char *pixel_area, double *coefficients, int dimSize)
{
    for (int i = 0; i < dimSize * dimSize; i++)
    {
        pixel_area[i] = (unsigned char) (pixel_area[i] * coefficients[i]);
    }
}

void multiply_each(double *pixel_area, double *coefficients, int dimSize)
{
    for (int i = 0; i < dimSize * dimSize; i++)
    {
        pixel_area[i] = pixel_area[i] * coefficients[i];
    }
}

double *sobelXkernel(int size)
{
    double *kernel_matrix = (double *) malloc(sizeof(double) * size * size);
    if (size == 3)
    {
        double coefficients[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};

        for (int i = 0; i < size * size; ++i)
        {
            kernel_matrix[i] = coefficients[i];
        }
        return kernel_matrix;
    }

    return nullptr;
}

double *sobelYkernel(int size)
{
    double *kernel_matrix = (double *) malloc(sizeof(double) * size * size);
    if (size == 3)
    {
        double coefficients[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

        for (int i = 0; i < size * size; ++i)
        {
            kernel_matrix[i] = coefficients[i];
        }
        return kernel_matrix;
    }
    return nullptr;
}

