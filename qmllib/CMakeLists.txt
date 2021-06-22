cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)
SET(OPENCV_DIR ./ CACHE PATH "OPENCV_DIR")

INCLUDE(${OPENCV_DIR}/OpenCVConfig.cmake)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories(include)

find_package(Qt5 COMPONENTS Core Gui Qml Quick QuickWidgets Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(BUILD_SHARED_LIBS True)

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "" FORCE)

add_definitions("-DIMAGETOOLQML_LIB")

set(HEADERS
        include/ImageToolQML.hpp
        include/PaintData.hpp
        include/ImageToolQML_global.h
        )
set(SOURCES
        src/ImageToolQML.cpp
        src/PaintData.cpp
        )

link_directories(${CMAKE_BINARY_DIR})

add_library(ImageToolQML  ${HEADERS} ${SOURCES})
target_link_libraries(ImageToolQML  Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick ${OpenCV_LIBS})

install (TARGETS ImageToolQML
        LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/qmllib/lib
        ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/qmllib/lib
        RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/qmllib/bin
        )

install(FILES ${HEADERS} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/qmllib/include/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../ImageToolQMLConfig.cmake DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/qmllib/)