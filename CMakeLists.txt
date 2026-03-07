cmake_minimum_required(VERSION 3.16)
project(SnapLite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



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

add_executable(SnapLite
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

if(APPLE)
    set_target_properties(SnapLite PROPERTIES MACOSX_BUNDLE TRUE)
endif()

target_include_directories(SnapLite PRIVATE src)

target_link_libraries(SnapLite PRIVATE Qt6::Widgets ${VTK_LIBRARIES})
vtk_module_autoinit(
    TARGETS SnapLite
    MODULES ${VTK_LIBRARIES}
)

if(WIN32)
    set(vtk_runtime_dirs "")
    foreach(vtk_target IN LISTS VTK_LIBRARIES)
        if(TARGET "${vtk_target}")
            get_target_property(vtk_imported_configs "${vtk_target}" IMPORTED_CONFIGURATIONS)
            foreach(vtk_config IN LISTS vtk_imported_configs)
                get_target_property(vtk_location "${vtk_target}" "IMPORTED_LOCATION_${vtk_config}")
                if(vtk_location AND NOT vtk_location MATCHES "-NOTFOUND$")
                    file(TO_CMAKE_PATH "${vtk_location}" vtk_location)
                    get_filename_component(vtk_runtime_dir "${vtk_location}" DIRECTORY)
                    list(APPEND vtk_runtime_dirs "${vtk_runtime_dir}")
                endif()
            endforeach()

            get_target_property(vtk_location "${vtk_target}" IMPORTED_LOCATION)
            if(vtk_location AND NOT vtk_location MATCHES "-NOTFOUND$")
                file(TO_CMAKE_PATH "${vtk_location}" vtk_location)
                get_filename_component(vtk_runtime_dir "${vtk_location}" DIRECTORY)
                list(APPEND vtk_runtime_dirs "${vtk_runtime_dir}")
            endif()
        endif()
    endforeach()
    list(REMOVE_DUPLICATES vtk_runtime_dirs)

    set(VTK_RUNTIME_DIRS "${vtk_runtime_dirs}")
    set(vtk_install_code [=[
        set(_vtk_runtime_dirs "@VTK_RUNTIME_DIRS@")
        set(_snaplite_executable "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/SnapLite@CMAKE_EXECUTABLE_SUFFIX@")
        set(_vtk_runtime_dependency_args
            PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
            POST_EXCLUDE_REGEXES
                ".*[/\\\\]System32[/\\\\].*"
                ".*[/\\\\]Windows[/\\\\]WinSxS[/\\\\].*"
                ".*[/\\\\]Qt6[^/\\\\]*\\.dll"
        )

        if(_vtk_runtime_dirs)
            list(PREPEND _vtk_runtime_dependency_args DIRECTORIES ${_vtk_runtime_dirs})
        endif()

        if(EXISTS "${_snaplite_executable}")
            file(GET_RUNTIME_DEPENDENCIES
                EXECUTABLES "${_snaplite_executable}"
                RESOLVED_DEPENDENCIES_VAR _vtk_resolved_deps
                UNRESOLVED_DEPENDENCIES_VAR _vtk_unresolved_deps
                ${_vtk_runtime_dependency_args}
            )

            foreach(_vtk_dep IN LISTS _vtk_resolved_deps)
                file(INSTALL
                    DESTINATION "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin"
                    TYPE SHARED_LIBRARY
                    FOLLOW_SYMLINK_CHAIN
                    FILES "${_vtk_dep}"
                )
            endforeach()

            if(_vtk_unresolved_deps)
                message(WARNING "Unresolved runtime dependencies while bundling VTK: ${_vtk_unresolved_deps}")
            endif()
        endif()
    ]=])
    string(CONFIGURE "${vtk_install_code}" vtk_install_code @ONLY)
endif()

if(APPLE)
    set(vtk_macos_runtime_dirs "")
    foreach(vtk_target IN LISTS VTK_LIBRARIES)
        if(TARGET "${vtk_target}")
            get_target_property(vtk_imported_configs "${vtk_target}" IMPORTED_CONFIGURATIONS)
            foreach(vtk_config IN LISTS vtk_imported_configs)
                get_target_property(vtk_location "${vtk_target}" "IMPORTED_LOCATION_${vtk_config}")
                if(vtk_location AND NOT vtk_location MATCHES "-NOTFOUND$")
                    file(TO_CMAKE_PATH "${vtk_location}" vtk_location)
                    get_filename_component(vtk_runtime_dir "${vtk_location}" DIRECTORY)
                    list(APPEND vtk_macos_runtime_dirs "${vtk_runtime_dir}")
                endif()
            endforeach()

            get_target_property(vtk_location "${vtk_target}" IMPORTED_LOCATION)
            if(vtk_location AND NOT vtk_location MATCHES "-NOTFOUND$")
                file(TO_CMAKE_PATH "${vtk_location}" vtk_location)
                get_filename_component(vtk_runtime_dir "${vtk_location}" DIRECTORY)
                list(APPEND vtk_macos_runtime_dirs "${vtk_runtime_dir}")
            endif()
        endif()
    endforeach()
    list(REMOVE_DUPLICATES vtk_macos_runtime_dirs)

    set(VTK_MACOS_RUNTIME_DIRS "${vtk_macos_runtime_dirs}")
    set(vtk_macos_install_code [=[
        include(BundleUtilities)
        set(_vtk_runtime_dirs "@VTK_MACOS_RUNTIME_DIRS@")
        set(_snaplite_bundle "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/SnapLite.app")

        if(EXISTS "${_snaplite_bundle}")
            fixup_bundle("${_snaplite_bundle}" "" "${_vtk_runtime_dirs}")
        endif()
    ]=])
    string(CONFIGURE "${vtk_macos_install_code}" vtk_macos_install_code @ONLY)
endif()




# 安装主程序
install(TARGETS SnapLite
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# 自动部署 Qt 运行库和插件
qt_generate_deploy_app_script(
    TARGET SnapLite
    OUTPUT_SCRIPT deploy_qt_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_qt_script})

if(WIN32)
    install(CODE "${vtk_install_code}")
endif()

if(APPLE)
    install(CODE "${vtk_macos_install_code}")
endif()

# CPack 基本信息
set(CPACK_PACKAGE_NAME "SnapLite")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")

include(CPack)
