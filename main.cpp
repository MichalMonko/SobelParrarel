#include <iostream>
#include <opencv2/opencv.hpp>
#include "include/ImageTransformation.h"
#include <time.h>
#include <mpi.h>

using namespace cv;
using namespace std;

unsigned char *makeGreyscaleCopy(const unsigned char *imageDataBegin, int imageDataSize, int channels);

int getClosestMultiplicant(int number, int multiplier);

static const double factorR = 0.3;
static const double factorG = 0.59;
static const double factorB = 0.11;

uchar *greyscaleCopy;

int main(int argc, char **argv)
{
    int *dataSize, *dataOffset, *rcv_dataSize, *rcv_dataOffset;

    uchar *greyscaleFragment;
    uchar *imageFragment;
    int imageWidth;
    int imageHeight;
    int channels;
    Mat originalImage;

    double start, end, time_passed;

    MPI_Init(NULL, NULL);

    int worldSize;
    int worldRank;

    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

    if (worldRank == 0)
    {
        start = MPI_Wtime();
    }
    MPI_Barrier(MPI_COMM_WORLD);

    dataSize = (int *) malloc(sizeof(int) * worldSize);
    rcv_dataSize = (int *) malloc(sizeof(int) * worldSize);
    dataOffset = (int *) malloc(sizeof(int) * worldSize);
    rcv_dataOffset = (int *) malloc(sizeof(int) * worldSize);

    if (argc < 2)
    {
        cerr << "Invalid arguments number" << endl;
        return -1;
    }

    if (worldRank == 0)
    {
        originalImage = imread(argv[1]);


        if (originalImage.empty() || originalImage.depth() != CV_8U)
        {
            cerr << "Image " << argv[0] << " is empty or type invalid" << endl;
            return -1;
        }

        imageWidth = originalImage.cols;
        imageHeight = originalImage.rows;
        channels = originalImage.channels();
        int imageSize = originalImage.cols * originalImage.rows * channels;
        int dataFragment = imageSize / worldSize;

        int allocatedData = 0;
        int dataChunk = 0;
        for (int i = 1; i < worldSize; ++i)
        {
            dataChunk = getClosestMultiplicant(dataFragment, channels);
            dataSize[i] = dataChunk;
            allocatedData += dataChunk;
        }

        dataSize[0] = imageSize - allocatedData;
        dataOffset[0] = 0;
        rcv_dataSize[0] = dataSize[0] / channels;
        rcv_dataOffset[0] = 0;

        for (int j = 1; j < worldSize; ++j)
        {
            rcv_dataSize[j] = dataSize[j] / channels;
            dataOffset[j] = dataOffset[j - 1] + dataSize[j - 1];
            rcv_dataOffset[j] = dataOffset[j] / channels;
        }

        greyscaleCopy = (uchar *) malloc(sizeof(uchar) * imageSize);
    }

    MPI_Bcast(dataSize, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(dataOffset, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(rcv_dataSize, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(rcv_dataOffset, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);


    imageFragment = (uchar *) malloc(sizeof(uchar *) * dataSize[worldRank]);

    cout << worldRank << " DataSize " << dataSize[worldRank] << " Data offset " << dataOffset[worldRank] << endl;
    cout << worldRank << " DataSizeReceive " << rcv_dataSize[worldRank] << " Data receive offset "
         << rcv_dataOffset[worldRank] << endl;

    MPI_Scatterv(originalImage.datastart, dataSize, dataOffset, MPI_UNSIGNED_CHAR, imageFragment, dataSize[worldRank],
                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    greyscaleFragment = makeGreyscaleCopy(imageFragment, dataSize[worldRank],
                                          channels);

    MPI_Gatherv(greyscaleFragment, rcv_dataSize[worldRank], MPI_UNSIGNED_CHAR, greyscaleCopy, rcv_dataSize,
                rcv_dataOffset,
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    if (worldRank == 0)
    {
        ImageDataClass image(greyscaleCopy, imageHeight, imageWidth, 1);
        cv::Mat greyscaleImage = Mat(image.numOfRows, image.numOfColumns, CV_8UC1, greyscaleCopy);

        auto *greyscaleCopy1 = (u_char *) malloc(image.numOfRows * image.numOfColumns * sizeof(u_char));
        auto *greyscaleCopy2 = (u_char *) malloc(image.numOfRows * image.numOfColumns * sizeof(u_char));

        memcpy(greyscaleCopy1, greyscaleCopy, (size_t) image.numOfRows * image.numOfColumns);
        memcpy(greyscaleCopy2, greyscaleCopy, (size_t) image.numOfRows * image.numOfColumns);

        auto *gradientImageX = new Mat(image.numOfRows, image.numOfColumns, CV_8UC1, greyscaleCopy1);
        auto *gradientImageY = new Mat(image.numOfRows, image.numOfColumns, CV_8UC1, greyscaleCopy2);

        auto *gradientX = new ImageDataClass(gradientImageX->data, image.numOfRows, image.numOfColumns, 1);
        auto *gradientY = new ImageDataClass(gradientImageY->data, image.numOfRows, image.numOfColumns, 1);

        auto *sobelMatrixX = new TransformationMatrix<double>(3, multiply_each, sobelXkernel);
        auto *sobelMatrixY = new TransformationMatrix<double>(3, multiply_each, sobelYkernel);


        gradientX->dataStart = convolve(gradientX, 3, sum_pixel_values_absolute, sobelMatrixX, ZERO_FILL);
        gradientY->dataStart = convolve(gradientY, 3, sum_pixel_values_absolute, sobelMatrixY, ZERO_FILL);

        cv::Mat sobelImage = Mat(image.numOfRows, image.numOfColumns, CV_8UC1);

        set_threshold(120);
        sobelImage.data = multiply_and_sqrt_each_pixel(gradientX, gradientY);

        delete gradientImageX;
        delete gradientImageY;
        delete sobelMatrixX;
        delete sobelMatrixY;
        free(greyscaleCopy1);
        free(greyscaleCopy2);

//        namedWindow("sobel");
//        imshow("sobel", sobelImage);
//        waitKey(0);
//        destroyAllWindows();

        if (!imwrite(argv[2], sobelImage))
        {
            cerr << "Cannot save converted image, please check filename and extension.\n";
        }

    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (worldRank == 0)
    {
        end = MPI_Wtime();
        time_passed = end - start;

        cout << "Time passed: " << time_passed << " seconds" << endl;
    }

    MPI_Finalize();

    return 0;

}

int getClosestMultiplicant(int number, int multiplier)
{
    int q = number / multiplier;
    return min((multiplier * q), (multiplier * (q + 1)));
}

unsigned char *
makeGreyscaleCopy(const unsigned char *imageDataBegin, int imageDataSize, int channels)
{
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

//int main(int argc, char **argv)
//{
//    if (argc < 2)
//    {
//        cerr << "Invalid arguments number" << endl;
//        return -1;
//    }
//
//    Mat originalImage = imread(argv[1]);
//
//    int imageHeight = originalImage.rows;
//    int imageWidth = originalImage.cols;
//
//    Mat medianedImage(imageHeight, imageWidth, CV_8UC3);
//    Mat averagedImage(imageHeight, imageWidth, CV_8UC3);
//
//    if (originalImage.empty() || originalImage.depth() != CV_8U)
//    {
//        cerr << "Image " << argv[0] << " is empty or type invalid" << endl;
//        return -1;
//    }
//
//    Mat bgr[3];
//    split(originalImage, bgr);
//
////OpenCV uses BGR color order!!!
//    ImageDataClass B(bgr[0].data, imageHeight, imageWidth, 1);
//    ImageDataClass G(bgr[1].data, imageHeight, imageWidth, 1);
//    ImageDataClass R(bgr[2].data, imageHeight, imageWidth, 1);
//
//    bgr[0].data = convolve(B, 5, median_filter, nullptr, NN_CLONE);
//    bgr[1].data = convolve(G, 5, median_filter, nullptr, NN_CLONE);
//    bgr[2].data = convolve(R, 5, median_filter, nullptr, NN_CLONE);
//
//    merge(bgr, 3, medianedImage);
//
//    Mat average_bgr[3];
//    split(originalImage, average_bgr);
//
////OpenCV uses BGR color order!!!
//    ImageDataClass aB(average_bgr[0].data, imageHeight, imageWidth, 1);
//    ImageDataClass aG(average_bgr[1].data, imageHeight, imageWidth, 1);
//    ImageDataClass aR(average_bgr[2].data, imageHeight, imageWidth, 1);
//
//    int kernelSize = 5;
//    auto *hat_kernel_matrix = new TransformationMatrix(kernelSize, multiply_each, hat_kernel);
//
//    average_bgr[0].data = convolve(aB, kernelSize, sum_pixel_values, hat_kernel_matrix, NN_CLONE);
//    average_bgr[1].data = convolve(aG, kernelSize, sum_pixel_values, hat_kernel_matrix, NN_CLONE);
//    average_bgr[2].data = convolve(aR, kernelSize, sum_pixel_values, hat_kernel_matrix, NN_CLONE);
//
//    delete hat_kernel_matrix;
//
//    merge(average_bgr, 3, averagedImage);
//
//    imwrite("leoMedian.jpg", medianedImage);
//    imwrite("leoAvg.jpg", averagedImage);
//
//    namedWindow("original");
//    namedWindow("medianFilter");
//    namedWindow("averageHat");
//    imshow("original", originalImage);
//    imshow("medianFilter", medianedImage);
//    imshow("averageHat", averagedImage);
//    waitKey(0);
//    destroyAllWindows();
//}


