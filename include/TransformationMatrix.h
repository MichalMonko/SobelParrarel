//
// Created by warchlak on 07.10.18.
//


template<typename T>
class TransformationMatrix
{
public:
    const int dimSize;
    double *coefficients;

    TransformationMatrix(int dimSize, void (*func)(T *, double *, int),
                         double *(*kernel_initializer)(int))
            : dimSize(dimSize)
    {
        function = func;
        coefficients = kernel_initializer(dimSize);
    }


    void apply_kernel(T *pixel_area, int dimSize)
    {
        function(pixel_area, coefficients, dimSize);
    }

    ~TransformationMatrix()
    {
        free(coefficients);
    }

private:
    void (*function)(T *, double *, int);
};
double *sobelXkernel(int size);

double *sobelYkernel(int size);

void multiply_each(unsigned char *pixel_area, double *coefficients, int dimSize);

void multiply_each(double *pixel_area, double *coefficients, int dimSize);

