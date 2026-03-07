cmake_minimum_required(VERSION 3.16)
project(QtSnapLite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VTK_DIR C:\\VTK-9.6.0\\build)


find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    IOImage
    RenderingCore
    RenderingVolume
    RenderingVolumeOpenGL2
    InteractionStyle
    GUISupportQt
)

set(CMAKE_AUTOMOC ON)

add_executable(QtSnapLite
    src/main.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/SliceView.h
    src/ui/SliceView.cpp
    src/data/VolumeData.h
    src/data/VolumeData.cpp
    src/rendering/VolumeRenderPlaceholder.h
    src/rendering/VolumeRenderPlaceholder.cpp
)

target_include_directories(QtSnapLite PRIVATE src)

target_link_libraries(QtSnapLite PRIVATE Qt6::Widgets ${VTK_LIBRARIES})
vtk_module_autoinit(
    TARGETS QtSnapLite
    MODULES ${VTK_LIBRARIES}
)
