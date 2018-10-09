//
// Created by warchlak on 07.10.18.
//

#ifndef SOBEL_TRANSFORMATIONMATRIX_H
#define SOBEL_TRANSFORMATIONMATRIX_H

template<const int dimSize, typename T>
class TransformationMatrix
{
public:
    const int dimensionSize;

    explicit TransformationMatrix(T *coefficientsArray)
            : dimensionSize(dimSize)
    {
        coefficients = coefficientsArray;
    }

    T *coefficients;

    const T valueAt(int rowIndex, int colIndex);

    const T getCoefficientsSum();

};

template
class TransformationMatrix<3, int>;

#endif //SOBEL_TRANSFORMATIONMATRIX_H
