//
// Created by warchlak on 08.10.18.
//
#include <stdexcept>
#include <assert.h>
#include <cmath>
#include "include/TransformationMatrix.h"


double *linspace(double start, double end, int numOfPoints)
{
    auto *spacing_start = (double *) malloc(numOfPoints * sizeof(double));
    double *spacing = spacing_start;

    double step = (end - start) / (numOfPoints - 1);
    double position = start;

    while (numOfPoints > 0)
    {
        *spacing = position;
        position += step;
        spacing++;
        numOfPoints--;
    }
    return spacing_start;
}

double *hat_function(double *spacing, int size)
{
    auto *func = (double *) malloc(sizeof(double) * size);
    for (int i = 0; i < size; i++)
    {
        func[i] = 1 - fabs(spacing[i]);
    }

    double sum = sum_coefficients(func, size);

    for (int i = 0; i < size; i++)
    {
        func[i] = (func[i] / sum);
    }

    return func;
}

double *transpose_and_multiply(double *vector, int size)
{
    auto *matrix = (double *) malloc(sizeof(double) * size * size);
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            double value = vector[i] * vector[j];
            int location = i * size + j;
            matrix[location] = value;
        }
    }
    return matrix;
}

double *hat_kernel(int size)
{
    double *spacing = linspace(-1.0f, 1.0f, size);
    double *hat_func = hat_function(spacing, size);
    double *hat_kernel = transpose_and_multiply(hat_func, size);

    free(spacing);
    free(hat_func);

    return hat_kernel;
}

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

double sum_coefficients(double *vector, int size)
{
    double sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += vector[i];
    }
    return sum;
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

