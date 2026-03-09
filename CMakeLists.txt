cmake_minimum_required(VERSION 3.21)
project(SnapLite VERSION 0.1.0 LANGUAGES CXX)

if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

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

set(SNAPLITE_SOURCES
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

add_executable(SnapLite ${SNAPLITE_SOURCES})

if(APPLE)
    set_target_properties(SnapLite PROPERTIES MACOSX_BUNDLE TRUE)
endif()

target_include_directories(SnapLite PRIVATE src)
target_link_libraries(SnapLite PRIVATE Qt6::Widgets ${VTK_LIBRARIES})

vtk_module_autoinit(
    TARGETS SnapLite
    MODULES ${VTK_LIBRARIES}
)

include(GNUInstallDirs)

if(WIN32)
    set(SNAPLITE_INSTALL_RUNTIME_DIR ".")
    set(SNAPLITE_RUNTIME_SEARCH_DIRS
        $<TARGET_FILE_DIR:Qt6::Core>
        $<TARGET_FILE_DIR:Qt6::Widgets>
    )

    foreach(vtk_module IN LISTS VTK_LIBRARIES)
        list(APPEND SNAPLITE_RUNTIME_SEARCH_DIRS $<TARGET_FILE_DIR:${vtk_module}>)
    endforeach()

    list(REMOVE_DUPLICATES SNAPLITE_RUNTIME_SEARCH_DIRS)
else()
    set(SNAPLITE_INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})
endif()

if(WIN32)
    install(TARGETS SnapLite
        RUNTIME_DEPENDENCIES
            DIRECTORIES ${SNAPLITE_RUNTIME_SEARCH_DIRS}
            PRE_EXCLUDE_REGEXES
                "api-ms-win-.*"
                "ext-ms-.*"
            POST_EXCLUDE_REGEXES
                ".*/[Ss][Yy][Ss][Tt][Ee][Mm]32/.*\\.dll"
                ".*/[Ss]ys[Ww][Oo][Ww]64/.*\\.dll"
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${SNAPLITE_INSTALL_RUNTIME_DIR}
    )

    if(TARGET Qt6::QWindowsIntegrationPlugin)
        install(IMPORTED_RUNTIME_ARTIFACTS Qt6::QWindowsIntegrationPlugin
            DESTINATION platforms
        )
    endif()

    if(TARGET Qt6::QModernWindowsStylePlugin)
        install(IMPORTED_RUNTIME_ARTIFACTS Qt6::QModernWindowsStylePlugin
            DESTINATION styles
        )
    endif()

    install(FILES README.md DESTINATION .)

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SNAPLITE_PACKAGE_ARCH x64)
    else()
        set(SNAPLITE_PACKAGE_ARCH x86)
    endif()

    set(CPACK_GENERATOR "ZIP")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
    set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-windows-${SNAPLITE_PACKAGE_ARCH}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
    include(CPack)
else()
    install(TARGETS SnapLite
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${SNAPLITE_INSTALL_RUNTIME_DIR}
    )
endif()
