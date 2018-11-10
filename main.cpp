#include <iostream>
#include <opencv2/opencv.hpp>
#include "ImageData.h"
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
        ImageData<unsigned char> image(greyscaleCopy, imageHeight, imageWidth, 1,
                                       CV_8UC1);
        cv::Mat greyscaleImage = Mat(image.numOfRows, image.numOfColumns, image.cvType, image.getData());

        int coefficients[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        TransformationMatrix<3, int> sobelX(coefficients);

        int coefficients_y[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
        TransformationMatrix<3, int> sobelY(coefficients_y);


        image.applyConvolutionKernels<3, int>(sobelX, sobelY);


        cv::Mat sobelImage = Mat(image.numOfRows, image.numOfColumns, image.cvType, image.getData());

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

