cmake_minimum_required(VERSION 3.16)

project(IpConfigTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ARAVIS REQUIRED aravis-0.10)

add_executable(${PROJECT_NAME}
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    NetworkCardTab.cpp
    NetworkCardTab.h
    NetworkCardTab.ui
    CameraTab.cpp
    CameraTab.h
    CameraTab.ui
    NetworkConfigurator.cpp
    NetworkConfigurator.h
    CameraConfigurator.cpp
    CameraConfigurator.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/include
    ${ARAVIS_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core Qt5::Widgets Qt5::Network ${ARAVIS_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE ${ARAVIS_CFLAGS_OTHER})
