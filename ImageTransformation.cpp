#include <cmath>
#include "include/ImageTransformation.h"

u_char THRESH = 50;

//Podnoszenie do kwaratu, sumowanie, pierwiastkowanie oraz progowanie dla dwóch obrazów
u_char *multiply_and_sqrt_each_pixel(ImageDataClass *img1, ImageDataClass *img2)
{
    int numOfRows = img1->numOfRows;
    int numOfColumns = img1->numOfColumns;

    //sprawdzenie czy rozmiary obu obrazów są sobie równe
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

    //dokonywanie operacji matematycznej sqrt(x^2 + y^2) a następnie progowanie w celu utworzenia
    //obrazu wynikowego
    for (int row = 0; row < numOfRows; row++)
    {
        for (int col = 0; col < numOfColumns; col++)
        {
            pixelValue1 = image_matrix_1[numOfColumns * row + col];
            pixelValue2 = image_matrix_2[numOfColumns * row + col];
            pixelValueResult = (u_char) sqrt(pixelValue1 * pixelValue1 + pixelValue2 * pixelValue2);
            pixelValueResult = (pixelValueResult > THRESH) ? (u_char) 255 : (u_char) 0;
            new_image[numOfColumns * row + col] = pixelValueResult;
        }
    }

    return new_image;
}

//wartość bezwględna sumy wszystkich pikseli w oknie
u_char sum_pixel_values_absolute(double *pixel_area, int dimSize)
{
    double sum = 0;
    for (int i = 0; i < (dimSize * dimSize); i++)
    {
        sum += pixel_area[i];
    }
    return (u_char) fabs(sum);
}

void set_threshold(u_char threshold)
{
    THRESH = threshold;
}


























