//
// Created by warchlak on 08.10.18.
//
#include <stdexcept>
#include <assert.h>
#include "TransformationMatrix.h"


template<const int dimSize, typename T>
const T TransformationMatrix<dimSize, T>::valueAt(int rowIndex, int colIndex)
{
    assert(rowIndex >= 0 || rowIndex < dimSize);
    assert(colIndex >= 0 || colIndex < dimSize);

    return coefficients[colIndex + dimSize * rowIndex];
}

template<const int dimSize, typename T>
const T TransformationMatrix<dimSize, T>::getCoefficientsSum()
{
    T sum = 0;
    for (int i = 0; i < dimSize * dimSize; i++)
    {
        sum += coefficients[i];
    }

    return sum;
}


