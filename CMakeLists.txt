cmake_minimum_required(VERSION 3.21)
project(SnapLite VERSION 0.1.0 LANGUAGES CXX)

if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

# qt_generate_deploy_app_script() 属于 Qt6::Core 提供的命令
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
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
    set_target_properties(SnapLite PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "SnapLite"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.xiangcen.snaplite"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        INSTALL_RPATH "@executable_path/../Frameworks"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.xiangcen.snaplite"
    )
endif()

target_include_directories(SnapLite PRIVATE src)
target_link_libraries(SnapLite PRIVATE Qt6::Widgets ${VTK_LIBRARIES})

vtk_module_autoinit(
    TARGETS SnapLite
    MODULES ${VTK_LIBRARIES}
)

include(GNUInstallDirs)

if(APPLE)
    set(SNAPLITE_INSTALL_BUNDLE_DIR ".")
    set(SNAPLITE_INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})
elseif(WIN32)
    set(SNAPLITE_INSTALL_BUNDLE_DIR ".")
    set(SNAPLITE_INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})
else()
    set(SNAPLITE_INSTALL_BUNDLE_DIR ${CMAKE_INSTALL_BINDIR})
    set(SNAPLITE_INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})
endif()

install(TARGETS SnapLite
    BUNDLE DESTINATION ${SNAPLITE_INSTALL_BUNDLE_DIR}
    RUNTIME DESTINATION ${SNAPLITE_INSTALL_RUNTIME_DIR}
)

# 让 Qt 在 install/package 阶段部署依赖
qt_generate_deploy_app_script(
    TARGET SnapLite
    OUTPUT_SCRIPT SNAPLITE_DEPLOY_SCRIPT
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${SNAPLITE_DEPLOY_SCRIPT})

# 关键：部署完成后重新签名
# 这里只用 "-"，表示本机可运行的 ad-hoc 签名，不需要开发者证书
if(APPLE)
    install(CODE "
        set(APP_PATH \"\${CMAKE_INSTALL_PREFIX}/SnapLite.app\")

        message(STATUS \"Re-signing \${APP_PATH} with ad-hoc signature\")

        execute_process(
            COMMAND /usr/bin/codesign --force --deep --sign - \"\${APP_PATH}\"
            RESULT_VARIABLE sign_result
            COMMAND_ECHO STDOUT
        )
        if(NOT sign_result EQUAL 0)
            message(FATAL_ERROR \"codesign failed for \${APP_PATH}\")
        endif()

        execute_process(
            COMMAND /usr/bin/codesign --verify --deep --strict --verbose=4 \"\${APP_PATH}\"
            RESULT_VARIABLE verify_result
            COMMAND_ECHO STDOUT
        )
        if(NOT verify_result EQUAL 0)
            message(FATAL_ERROR \"codesign verify failed for \${APP_PATH}\")
        endif()
    ")
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "SnapLite")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_MONOLITHIC_INSTALL ON)

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME} ${PROJECT_VERSION}")
    set(CPACK_SYSTEM_NAME "macos-${CMAKE_SYSTEM_PROCESSOR}")
elseif(WIN32)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_SYSTEM_NAME "windows-${CMAKE_SYSTEM_PROCESSOR}")
else()
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}"
)

include(CPack)