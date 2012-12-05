cmake_minimum_required(VERSION 2.6)
set(CMAKE_AUTOMOC TRUE)
project(quadtree)

find_package(OpenMP REQUIRED)
find_package(Qt4 REQUIRED QtGui QtCore)
find_package(Boost)
include(${QT_USE_FILE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -Wall -std=c++0x")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OpenMP_CXX_FLAGS}")

include_directories(${Boost_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_UI(UI_SRCS mainwindow.ui)

add_executable(quadtree main.cpp graphicsscene.cpp mainwindow.cpp quadtree.h particle.h ${UI_SRCS})
#add_executable(quadtree main.cpp quadtree.h ${UI_SRCS})
target_link_libraries(quadtree ${QT_LIBRARIES})
