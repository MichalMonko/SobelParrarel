cmake_minimum_required(VERSION 3.12)
project(Sobel)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
find_package(MPI REQUIRED)

add_executable(Sobel main.cpp include/TransformationMatrix.h TransformationMatrix.cpp include/ImageTransformation.h ImageTransformation.cpp include/ImageDataClass.h ImageDataClass.cpp include/BorderType.h)
target_link_libraries(Sobel ${OpenCV_LIBS} ${MPI_LIBRARIES})

