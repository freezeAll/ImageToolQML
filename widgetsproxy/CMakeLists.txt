cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)
SET(OPENCV_DIR ./ CACHE PATH "OPENCV_DIR")

INCLUDE(${OPENCV_DIR}/OpenCVConfig.cmake)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories(include)
include_directories(../qmllib/include)

find_package(Qt5 COMPONENTS Core Gui Qml Quick QuickWidgets Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(BUILD_SHARED_LIBS True)

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "" FORCE)

add_definitions("-DIMAGETOOL_LIB")

set(WIDGETSOURCES
        src/ImageTool.cpp
)

set(WIDGETHEADERS
        include/ImageTool.hpp
        )
set(WIDGETRESOURCE
        resource/ImageTool.qrc
        )


add_definitions("-DIMAGETOOL_LIB")

add_library(ImageTool  ${WIDGETSOURCES} ${WIDGETHEADERS} ${WIDGETRESOURCE})
target_link_libraries(ImageTool Qt5::Core Qt5::Gui Qt5::Qml Qt5::QuickWidgets ImageToolQML ${OpenCV_LIBS})

install (TARGETS ImageTool
        LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/widgetsproxy/lib
        ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/widgetsproxy/lib
        RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/widgetsproxy/bin
        )

install(FILES ${WIDGETHEADERS} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/widgetsproxy/include/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../ImageToolQMLConfig.cmake DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../install/widgetsproxy)