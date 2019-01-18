#include "BorderType.h"
#include "ImageDataClass.h"
#include "TransformationMatrix.h"

#ifndef SOBEL_IMAGETRANSFORMATION_H
#define SOBEL_IMAGETRANSFORMATION_H


//operacje realizowane na każdym pikselu
u_char *multiply_and_sqrt_each_pixel(ImageDataClass *img1, ImageDataClass *img2);

u_char sum_pixel_values_absolute(double pixel_area[], int dimSize);

//ustawienie progowania dla operatarów Sobela
void set_threshold(u_char threshold);

/*operacja splotu
 *
 * imageData określa obraz na którym dokonywany jest splot
 *
 * dimSize określa rozmiar okna
 *
 * kernel określa jądro przekształcenia splotowego (operator Sobela X albo Y ale można też zadać inne jądro)
 *
 * transformation określa operację matematyczną przyjmującą jako argument
 * macierz o rozmiarach dimSize x dimSize i zwracającą pojedyńczą wartość
 *
 * kernel oraz transformation są argumentami opcjonalnymi co pozwala funkcji na dokonanie również innych
 * przekształceń niż standardowy splot.
 *
 *border_type określa sposób minimalizacji efektów brzegowych.
 *ZERO_FILL oznaczna wypełnienie zerami, NN_CLONE oznacza kopiowanie najbliższego piksela
 *


*/
template<typename T>
unsigned char *convolve(ImageDataClass *imageData, int dimSize, unsigned char (*transformation)(T *, int),
                        TransformationMatrix<T> *kernel, BORDER_TYPE border_type)
{
    int numOfRows = imageData->numOfRows;
    int numOfColumns = imageData->numOfColumns;

    //dimSize oznacza wielkość okna
    T pixel_area[dimSize * dimSize];

    auto *processedImage = (unsigned char *) malloc(numOfColumns * numOfRows * sizeof(unsigned char));

    int topLeftPixelRow, topLeftPixelColumn, offset_to_border;

    //Zmienna określa dystans od piksela środkowego do lewej krawędzi okna
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
                    //Przypisywanie pikseli do okna
                    pixel_area[rowOffset * dimSize + columnOffset] =
                            (T) imageData->getPixelValueRelativeTo(topLeftPixelRow, topLeftPixelColumn, rowOffset,
                                                                   columnOffset, border_type);
                }
            }

            if (kernel != nullptr)
            {
                //Wymnażanie okna z jądrem przekształcenia splotowego
                kernel->apply_kernel(pixel_area, dimSize);
            }
            if (transformation != nullptr)
            {
                //Wykonanie operacji (zadanej jako wskaźnik na funkcję) na oknie i
                // przypisanie wyniku do rozpatrywanego piksela
                processedImage[row * numOfColumns + col] = transformation(pixel_area, dimSize);
            }
        }
    }
    return processedImage;
}


#endif //SOBEL_IMAGETRANSFORMATION_H
