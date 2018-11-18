//
// Created by warchlak on 16.11.18.
//

#ifndef SOBEL_IMAGETRANSFORMATION_H
#define SOBEL_IMAGETRANSFORMATION_H

#endif //SOBEL_IMAGETRANSFORMATION_H

unsigned char *convolve(ImageData imageData,
                        unsigned char *(transformation)(unsigned char *pixels, const int size),
                        BORDER_TYPE border_type);

template<typename T>
T median_filter(T *pixel_area, int dimSize);

template<typename T>
T insertion_sort(T *pixel_area, int dimSize);
