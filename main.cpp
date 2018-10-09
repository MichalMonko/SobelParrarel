#include <iostream>
#include <opencv2/opencv.hpp>
#include "ImageData.h"

using namespace cv;
using namespace std;

unsigned char *makeGreyscaleCopy(const unsigned char *imageDataBegin, int imageHeight, int imageWidth, int channels);

static const double factorR = 0.3;
static const double factorG = 0.59;
static const double factorB = 0.11;

int main(int argc, char **argv)
{
    Mat originalImage = imread("/home/warchlak/CLionProjects/Sobel/valve.PNG");

    if (originalImage.empty() || originalImage.depth() != CV_8U)
    {
        cout << "Image is empty or type invalid";
        return -1;
    }

    int imageWidth = originalImage.cols;
    int imageHeight = originalImage.rows;


    unsigned char *greyscaleCopy = makeGreyscaleCopy(originalImage.datastart, imageHeight, imageWidth,
                                                     originalImage.channels());


//    cv::Mat greyscaleImage = Mat(imageHeight, imageWidth, CV_8UC1, greyscaleCopy);
//
    ImageData<unsigned char> image(greyscaleCopy, imageHeight, imageWidth, 1,
                                   CV_8UC1);

    int coefficients[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    TransformationMatrix<3, int> sobelX(coefficients);

    int coefficients_y[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
    TransformationMatrix<3, int> sobelY(coefficients_y);

    image.applyConvolutionKernels<3, int>(sobelX, sobelY);

    cv::Mat matImage = Mat(image.numOfRows, image.numOfColumns, image.cvType, image.getData());

    String windowName = "original";
    namedWindow(windowName);
    imshow(windowName, originalImage);
    waitKey(0);

//    windowName = "grey";
//    namedWindow(windowName, WINDOW_NORMAL);
//    imshow(windowName, greyscaleImage);
//    waitKey(0);

    windowName = "SOBEL";
    namedWindow(windowName);
    imshow(windowName, matImage);
    waitKey(0);

    destroyAllWindows();

    return 0;

}

unsigned char *
makeGreyscaleCopy(const unsigned char *imageDataBegin, int imageHeight, int imageWidth, int channels)
{
    int imageDataSize = (imageHeight * imageWidth);
    auto *const greyscaleCopy = (unsigned char *) malloc(imageDataSize * sizeof(unsigned char));

    if (channels == 1)
    {
        memcpy(greyscaleCopy, imageDataBegin, imageDataSize * sizeof(unsigned char));
        return greyscaleCopy;
    }

    const unsigned char *imageData = imageDataBegin;
    unsigned char *greyscaleCopyIterator = greyscaleCopy;

    unsigned char imagePixelValue = 0;

    unsigned char B;
    unsigned char G;
    unsigned char R;

    for (int i = 0; i < imageDataSize * channels; i += channels)
    {
        R = imageData[i + 2];
        G = imageData[i + 1];
        B = imageData[i];

        imagePixelValue = (unsigned char) (R * factorR + G * factorG + B * factorB);
        *greyscaleCopyIterator = imagePixelValue;
        greyscaleCopyIterator++;
    }

    return greyscaleCopy;
}

