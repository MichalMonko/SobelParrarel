#include <iostream>
#include <opencv2/opencv.hpp>
#include "include/ImageTransformation.h"
#include <time.h>
#include <mpi.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
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

    MPI_Bcast(&imageWidth, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&imageHeight, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(dataSize, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(dataOffset, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(rcv_dataSize, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(rcv_dataOffset, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);


    imageFragment = (uchar *) malloc(sizeof(uchar *) * dataSize[worldRank]);

    MPI_Scatterv(originalImage.datastart, dataSize, dataOffset, MPI_UNSIGNED_CHAR, imageFragment, dataSize[worldRank],
                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    greyscaleFragment = makeGreyscaleCopy(imageFragment, dataSize[worldRank],
                                          channels);

    MPI_Gatherv(greyscaleFragment, rcv_dataSize[worldRank], MPI_UNSIGNED_CHAR, greyscaleCopy, rcv_dataSize,
                rcv_dataOffset,
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    uchar *sobelData = greyscaleCopy;

    auto *data_to_send_index = (int *) malloc(sizeof(int) * worldSize);
    auto *pixels_to_receive = (int *) malloc(sizeof(int) * worldSize);
    auto *pixels_to_send_back = (int *) malloc(sizeof(int) * worldSize);
    auto *num_of_rows = (int *) malloc(sizeof(int) * worldSize);
    auto *data_receive_index = (int *) malloc(sizeof(int) * worldSize);
    auto *send_back_starting_index = (int *) malloc(sizeof(int) * worldSize);

    if (worldRank == 0)
    {
        int rows_per_core = imageHeight / worldSize;
        int remaining_rows = imageHeight % worldSize;

        for (int i = 0; i < worldSize; ++i)
        {
            if (i == 0)
            {
                data_to_send_index[i] = 0;
                num_of_rows[i] = rows_per_core + remaining_rows + 1;
                pixels_to_receive[i] = num_of_rows[i] * imageWidth;
                pixels_to_send_back[i] = (num_of_rows[i] - 1) * imageWidth;
                data_receive_index[i] = 0;
                send_back_starting_index[i] = 0;
                continue;
            }
            if (i == (worldSize - 1))
            {
                data_to_send_index[i] = (i * rows_per_core - 1) * imageWidth;
                num_of_rows[i] = rows_per_core + 1;
                pixels_to_receive[i] = num_of_rows[i] * imageWidth;
                pixels_to_send_back[i] = (num_of_rows[i]) * imageWidth;
                data_receive_index[i] = (i * rows_per_core) * imageWidth;
                send_back_starting_index[i] = imageWidth;
                continue;
            }
            data_to_send_index[i] = (i * rows_per_core - 1) * imageWidth;
            num_of_rows[i] = rows_per_core + 2;
            pixels_to_receive[i] = num_of_rows[i] * imageWidth;
            pixels_to_send_back[i] = (num_of_rows[i] - 1) * imageWidth;
            data_receive_index[i] = (i * rows_per_core) * imageWidth;
            send_back_starting_index[i] = imageWidth;
        }
    }

    MPI_Bcast(data_to_send_index, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(pixels_to_receive, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(pixels_to_send_back, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(num_of_rows, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(data_receive_index, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(send_back_starting_index, worldSize, MPI_INT, 0, MPI_COMM_WORLD);

    free(greyscaleFragment);
    auto *sobelFragment = (uchar *) malloc(sizeof(uchar) * num_of_rows[worldRank] * imageWidth);
    auto *sobelFragmentX = (uchar *) malloc(sizeof(uchar) * num_of_rows[worldRank] * imageWidth);
    auto *sobelFragmentY = (uchar *) malloc(sizeof(uchar) * num_of_rows[worldRank] * imageWidth);

    if (worldRank != 0)
    {
        MPI_Recv(sobelFragment, pixels_to_receive[worldRank], MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }

    if (worldRank == 0)
    {
        for (int i = 1; i < worldSize; i++)
        {
            MPI_Send(greyscaleCopy + data_to_send_index[i], pixels_to_receive[i], MPI_UNSIGNED_CHAR, i, 0,
                     MPI_COMM_WORLD);
        }
        memcpy(sobelFragment, greyscaleCopy, static_cast<size_t>(pixels_to_receive[worldRank]));
    }

    memcpy(sobelFragmentX, sobelFragment, static_cast<size_t>(pixels_to_receive[worldRank]));
    memcpy(sobelFragmentY, sobelFragment, static_cast<size_t>(pixels_to_receive[worldRank]));

    auto *gradientX = new ImageDataClass(sobelFragmentX, num_of_rows[worldRank], imageWidth, 1);
    auto *gradientY = new ImageDataClass(sobelFragmentY, num_of_rows[worldRank], imageWidth, 1);

    auto *sobelMatrixX = new TransformationMatrix<double>(3, multiply_each, sobelXkernel);
    auto *sobelMatrixY = new TransformationMatrix<double>(3, multiply_each, sobelYkernel);

    gradientX->dataStart = convolve(gradientX, 3, sum_pixel_values_absolute, sobelMatrixX, ZERO_FILL);
    gradientY->dataStart = convolve(gradientY, 3, sum_pixel_values_absolute, sobelMatrixY, ZERO_FILL);

    set_threshold(50);
    sobelFragment = multiply_and_sqrt_each_pixel(gradientX, gradientY);

    if (worldRank != 0)
    {
        MPI_Send(sobelFragment + send_back_starting_index[worldRank], pixels_to_send_back[worldRank], MPI_UNSIGNED_CHAR,
                 0, 0, MPI_COMM_WORLD);
    }

    if (worldRank == 0)
    {
        memcpy(sobelData, sobelFragment, static_cast<size_t>(pixels_to_send_back[worldRank]));
        for (int i = 1; i < worldSize; ++i)
        {
            MPI_Recv(sobelData + data_receive_index[i], pixels_to_send_back[i], MPI_UNSIGNED_CHAR,
                     i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    if (worldRank == 0)
    {
        end = MPI_Wtime();
        time_passed = end - start;
        cout << "Time passed: " << time_passed << " seconds" << endl;

        cv::Mat sobelImage = Mat(imageHeight, imageWidth, CV_8UC1, sobelData);

//        namedWindow("sobel");
//        imshow("sobel", sobelImage);
//        waitKey(0);
//        destroyAllWindows();

        if (!imwrite(argv[2], sobelImage))
        {
            cerr << "Cannot save converted image, please check filename and extension.\n";
        }

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

#pragma clang diagnostic pop