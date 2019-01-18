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

//Stałe Używane do konersji na skalę szarości
static const double factorR = 0.3;
static const double factorG = 0.59;
static const double factorB = 0.11;
//Parametr przetwarzania określający wymaganą zmianę intensywności do uznania jej jako krawędź
static unsigned char THRESH = 50;

//Wskaźnik na obraz w skali szarości
uchar *greyscaleCopy;

int main(int argc, char **argv)
{
    //Wymagane są dwa argumenty: ścieżka do obrazu źródłowego oraz wynikowego, opcjonalnie próg
    if (argc < 2)
    {
        cerr << "Invalid arguments number, correct syntax:" << endl <<
             "\tSobel src_img_path result_img_path [threshold(1-255)]" << endl
             << "threshold is optional" << endl;
        return -1;
    }

    //Algorytm w pierwszej części przekształca obraz na skalę szarości. Obraz reprezentowany jest jako tablica

    //Tablice wymagana przez funkcje MPI do określenia który proces powinien otrzymać jakie dane, szerzej omównione dalej
    int *dataSize, *dataOffset, *rcv_dataSize, *rcv_dataOffset;

    //imageFragment określa wskaźnik na pamięć zawierającą fragment obrazu otrzymany przed dany proces,
    // analogicznie greyscaleFragment stanowi wskaźnik na obraz przetworzony do skali szarości przez dany proces

    uchar *greyscaleFragment;
    uchar *imageFragment;

    int imageWidth;
    int imageHeight;
    int channels;

    //Mat stanowi klasę biblioteki OPEN_CV reprezentującą obraz
    Mat originalImage;

    //zmienne wykorzystywane do pomiariu czasu
    double start, end, time_passed;

    //Inicjalizacja Przetwarzania równolegółego wymagana przez MPI
    MPI_Init(NULL, NULL);

    //worldSize określa ilość procesów obsłógiwanych przez MPI w ramach jednego komunikatora, worldRank stanowi ID
    //danego procesu w komunikatorze. W programie wykorzystywany jest jeden globalny komunikator MPI_COMM_WORLD
    int worldSize;
    int worldRank;

    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

    //Alokacja pamięci dla zmiennych kontrolnych potrzebnych do realizacji przekazania danych
    dataSize = (int *) malloc(sizeof(int) * worldSize);
    rcv_dataSize = (int *) malloc(sizeof(int) * worldSize);
    dataOffset = (int *) malloc(sizeof(int) * worldSize);
    rcv_dataOffset = (int *) malloc(sizeof(int) * worldSize);

    //Opcjonalny argument 3 (próg)
    if (argv[3])
    {
        char *char_end;
        THRESH = static_cast<unsigned char>(strtol(argv[3], &char_end, 10));
        //Ustawienie wartości progowania określającej jak duża musi być zmiana intensywności aby
        //została ona uznana za krawędź
        set_threshold(THRESH);
        if (THRESH == 0)
        {
            cerr << "Invalid threshold number, pick number from 1 to 255 (Default: 50)" << endl;
            return -1;
        }
    }

    //Synchronizacja procesów
    MPI_Barrier(MPI_COMM_WORLD);

    if (worldRank == 0)
    {

        //Wczytywanie obrazu zrealizowane przy pomocy biblioteki OPEN_CV wraz ze sprawdzeniem poprawności
        originalImage = imread(argv[1]);
        if (originalImage.empty() || originalImage.depth() != CV_8U)
        {
            cerr << "Image " << argv[0] << " is empty or type invalid" << endl;
            return -1;
        }

        if (worldRank == 0)
        {
            //Rozpoczęcie pomiaru czasu przez proces zarządzjący, gdyż on skończy się ostatni
            start = MPI_Wtime();
        }

        imageWidth = originalImage.cols;
        imageHeight = originalImage.rows;
        channels = originalImage.channels();

        /*Obliczenie ilości bajtów potrzebnych do reprezentacji obrazu w przestrzeni RGB i podzielenie
        tej wartości przez ilość procesów w celu określenia ilości danych na proces
         */
        int imageSize = originalImage.cols * originalImage.rows * channels;
        int dataFragment = imageSize / worldSize;

        /*Jako że obraz reprezentowany jest jako ciągły obszar pamięci o następujących po sobie wartościach R,G,B
        danego piksela, w celu zapewnienia poprawności obliczeń należy upewnić się że każdy proces dostanie ilość danych
         stonwiącą wielokrotność liczby 3.

         allocated Data określa ilość danych które zostały przydzielone do wszystkich procesów.

         dataChunk określa ilość danych przekazanych do danego procesu. Jest ona równa najbliższemu dzielnikowi
         liczby 3 względem wartości dataFragment.
         */
        int allocatedData = 0;
        int dataChunk = 0;

        /*Iteracja zaczyna się od procesu 1, proces 0 (zarządzający) przejmuje resztę danych
         * co w praktyce oznacza że dostaje ich nieco więcej niż inne procesy, jednak jest to marginalna nadwyżka
         */
        for (int i = 1; i < worldSize; ++i)
        {
            dataChunk = getClosestMultiplicant(dataFragment, channels);
            dataSize[i] = dataChunk;
            allocatedData += dataChunk;
        }

        /*Proces 0 dostaje resztę nieprzydzielonych danych z początku obrazu (dataOffset określa miejsce od którego
        * pobierane będą dane)
         * rcv_dataSize określa ile danych ma zwrócić dany proces. W tym przypadku wartości RGB konwertowane są do
         * skali szarości, a więc proces ma zwrócić 3 razy mniej danych niż dostał
        */

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

    /*Obraz wczytywał proces 0, pozostałe procesy nie mają o nim żadnych danych.
    * MPI_Bcast wysyła dane potrzebne do kontroli przetwarzania i przepływu do wszystkich procesów
    */
    MPI_Bcast(&imageWidth, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&imageHeight, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(dataSize, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(dataOffset, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(rcv_dataSize, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(rcv_dataOffset, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);


    //Każdy proces alokuje pamięć na swój fragment przetworzonego obrazu
    imageFragment = (uchar *) malloc(sizeof(uchar *) * dataSize[worldRank]);

    /*Przesyłanie fragmentów obrazu do procesów. Ilość danych oraz ich lokalizacja ustalana jest na podstawie wcześniej
    * wyznaczonych tablic dataSize oraz dataOffset
    */
    MPI_Scatterv(originalImage.datastart, dataSize, dataOffset, MPI_UNSIGNED_CHAR, imageFragment, dataSize[worldRank],
                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    //Funkcja konwertująca obraz RGB na skalę szarości
    greyscaleFragment = makeGreyscaleCopy(imageFragment, dataSize[worldRank],
                                          channels);

    /*Odsyłanie przekonwertowanych fragmentów obrazu do procsu nadzorujacego (proces 0)
     * oraz łączenie fragmentów w wynikowy obraz greyscaleCopy
     */
    MPI_Gatherv(greyscaleFragment, rcv_dataSize[worldRank], MPI_UNSIGNED_CHAR, greyscaleCopy, rcv_dataSize,
                rcv_dataOffset,
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    uchar *sobelData = greyscaleCopy;
    free(greyscaleFragment);

    /*Drugim etapem algorytmu jest zastosowanie operatorów Sobela do obrazu w skali szarości.
     * Jako że każdy proces musi otrzymać część danych wspólnych z innym procesem (na połączeniu wierszy obrazu)
     * nie można bezpośrednio skorzystać z funkcji MPI_Scatterv oraz MPI_Gatherv.
     * W celu rozwiązania tego problemu dane do każdego procesu przesyłano osobno za pomocą funkcji MPI_Send oraz
     * MPI_Recv.
     * W celu określenia jaką ilość danych oraz z jakiego miejsca przesłać do poszczególnych procesów, wymagane jest
     * stworzenie algorytmu podziału danych spełniającego warunki wymagane do integralności obrazu, a mianowicie
     * 1. Każdy proces musi otrzymać całkowitą ilość wierszy
     * 2. Procesy zawierające wiersze które nie są wierszem pierwszym i ostatnim muszą otrzymać dane o wierszu
     * poprzednim/ następnym w celu realizacji operacji splotu (wielkość okna wynosiła 3).
     */

    //tablice kontrolne określające ilość danych i ich lokalizacje dla każdego procesu
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
            /*Pierwszy proces otrzymuje jeden dodatkowy wiersz (pierwszy wiersz jest również pierwszym wierszem
            * obrazu więc nie wymaga dodatkowych danych)
            * Wiersz ten otrzymuje też nadmiarowe wiersze (W przypadku gdy ilość wieszy obrazu nie jest podzielna
             * przez ilość procesów
             */
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

            /*Ostatni proces otrzymuje nadamiarowy wiersz na początku, ostatni wiersz jest również ostatnim wierszem
             * całego obrazu, więc po nim nie ma dodatkowych danych
             */
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

            //Pozostałe wiersze otrzymują dodatkowy wiersz na początku oraz na końcu
            data_to_send_index[i] = (i * rows_per_core - 1) * imageWidth;
            num_of_rows[i] = rows_per_core + 2;
            pixels_to_receive[i] = num_of_rows[i] * imageWidth;
            pixels_to_send_back[i] = (num_of_rows[i] - 1) * imageWidth;
            data_receive_index[i] = (i * rows_per_core) * imageWidth;
            send_back_starting_index[i] = imageWidth;
        }
    }

    //Przesyałenia danych kontrolnych do wszystkich procesów
    MPI_Bcast(data_to_send_index, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(pixels_to_receive, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(pixels_to_send_back, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(num_of_rows, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(data_receive_index, worldSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(send_back_starting_index, worldSize, MPI_INT, 0, MPI_COMM_WORLD);

    /* Operatory Sobela wymagają dwóch splotów (osobne poszukiwanie krawędzi w płaszczyźnie X i Y)
     * ostateczny wynik obliczany jest na podstawie obu obliczonych splotów
     */
    auto *sobelFragment = (uchar *) malloc(sizeof(uchar) * num_of_rows[worldRank] * imageWidth);
    auto *sobelFragmentX = (uchar *) malloc(sizeof(uchar) * num_of_rows[worldRank] * imageWidth);
    auto *sobelFragmentY = (uchar *) malloc(sizeof(uchar) * num_of_rows[worldRank] * imageWidth);

    //Procesy inne niż 0 czekają na dane
    if (worldRank != 0)
    {
        MPI_Recv(sobelFragment, pixels_to_receive[worldRank], MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }

    //Proces 0 wysyła dane
    if (worldRank == 0)
    {
        for (int i = 1; i < worldSize; i++)
        {
            MPI_Send(greyscaleCopy + data_to_send_index[i], pixels_to_receive[i], MPI_UNSIGNED_CHAR, i, 0,
                     MPI_COMM_WORLD);
        }
        //Proces 0 też bierze udział w obliczeniach, posiada on jednak cały obraz daltego wystarczy że
        // przekopiuje dane bez użycia MPI
        memcpy(sobelFragment, greyscaleCopy, static_cast<size_t>(pixels_to_receive[worldRank]));
    }

    memcpy(sobelFragmentX, sobelFragment, static_cast<size_t>(pixels_to_receive[worldRank]));
    memcpy(sobelFragmentY, sobelFragment, static_cast<size_t>(pixels_to_receive[worldRank]));

    //Inicjalizacja klas zawierających obrazy stanowiące wyniki przetwarzania operatorami Sobela
    //w płaszczyźnie X oraz Y
    auto *gradientX = new ImageDataClass(sobelFragmentX, num_of_rows[worldRank], imageWidth, 1);
    auto *gradientY = new ImageDataClass(sobelFragmentY, num_of_rows[worldRank], imageWidth, 1);

    //Inicjalizacja klas reprezentujących jądra przekształcenia (operatory Sobela) w płaszczyźnie
    //X oraz Y (rózne jądra dla płaszczyzn).
    auto *sobelMatrixX = new TransformationMatrix<double>(3, multiply_each, sobelXkernel);
    auto *sobelMatrixY = new TransformationMatrix<double>(3, multiply_each, sobelYkernel);

    //Realizacja operacji splotu
    gradientX->dataStart = convolve(gradientX, 3, sum_pixel_values_absolute, sobelMatrixX, ZERO_FILL);
    gradientY->dataStart = convolve(gradientY, 3, sum_pixel_values_absolute, sobelMatrixY, ZERO_FILL);

    //Przekształcenie wyników dla obu płaszczyzn w jeden obraz wynikowy
    sobelFragment = multiply_and_sqrt_each_pixel(gradientX, gradientY);

    //Wysyłanie przetworzonych danych przez procesy i odbieranie ich przez proces 0
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

    //Zakonczenie pomiaru czasu oraz zapisanie obrazu wynikowego
    if (worldRank == 0)
    {
        end = MPI_Wtime();
        time_passed = end - start;
        cout << "Time passed: " << time_passed << " seconds" << endl;

        cv::Mat sobelImage = Mat(imageHeight, imageWidth, CV_8UC1, sobelData);

        if (!imwrite(argv[2], sobelImage))
        {
            cerr << "Cannot save converted image, please check filename and extension.\n";
        }

    }

    //Czyszczenie pamięci oraz kończenie MPI
    delete gradientX;
    delete gradientY;
    delete sobelMatrixX;
    delete sobelMatrixY;
    free(sobelFragmentX);
    free(sobelFragmentY);
    free(sobelFragment);
    free(sobelData);

    MPI_Finalize();

    return 0;
}

int getClosestMultiplicant(int number, int multiplier)
{
    int q = number / multiplier;
    return min((multiplier * q), (multiplier * (q + 1)));
}

//Funkcja przelicza obraz na skalę szarości
unsigned char *
makeGreyscaleCopy(const unsigned char *imageDataBegin, int imageDataSize, int channels)
{
    auto *const greyscaleCopy = (unsigned char *) malloc(imageDataSize * sizeof(unsigned char));

//Jeżeli obraz ma tylko jeden kanał to znaczy że jest już w skali szarości, wymagane jest jedynie kopiowanie
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
        //W bilbiotece OPEN_CV obraz reprezentowany jest w postaci BGR
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