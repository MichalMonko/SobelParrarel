cmake_minimum_required(VERSION 3.12)
project(Sobel)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
add_executable(Sobel main.cpp ImageData.h ImageData.cpp TransformationMatrix.h GaussianKernel.h TransformationMatrix.cpp)
target_link_libraries(Sobel ${OpenCV_LIBS})

