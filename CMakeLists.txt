cmake_minimum_required(VERSION 3.16)

# 设置vcpkg工具链文件路径
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{GITHUB_ACTIONS})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{GITHUB_WORKSPACE}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    else()
        set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    endif()
endif()

project(MusicPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt自动化设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt组件
find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
)
find_package(taglib CONFIG REQUIRED)

# 添加可执行文件
add_executable(MusicPlayer
        main.cpp
        musicplayer.cpp
        musicplayer.h
        musicplayer.ui
        restarhalf.hpp
        res.qrc
        app_icon.rc
        restarhalf.hpp
)

# 链接Qt库和TagLib
target_link_libraries(MusicPlayer
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
        TagLib::tag
        TagLib::tag_c
)

# 设置输出目录
set_target_properties(MusicPlayer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release
)

# 仅在Windows且非GitHub Actions环境下复制DLL
if(WIN32 AND NOT DEFINED ENV{GITHUB_ACTIONS})
    # 本地开发环境的DLL复制逻辑
    if(EXISTS "${Qt6_DIR}/../../../bin")
        set(QT_INSTALL_PATH "${Qt6_DIR}/../../../bin")
    elseif(EXISTS "${Qt6_DIR}/../../bin")
        set(QT_INSTALL_PATH "${Qt6_DIR}/../../bin")
    endif()

    if(QT_INSTALL_PATH)
        # 复制Qt DLL到输出目录
        foreach(QT_LIB Core Gui Widgets Multimedia Network)
            if(EXISTS "${QT_INSTALL_PATH}/Qt6${QT_LIB}.dll")
                add_custom_command(TARGET MusicPlayer POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT_INSTALL_PATH}/Qt6${QT_LIB}.dll"
                    "$<TARGET_FILE_DIR:MusicPlayer>")
            endif()
        endforeach()

        # 复制插件目录
        if(EXISTS "${QT_INSTALL_PATH}/../plugins")
            add_custom_command(TARGET MusicPlayer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${QT_INSTALL_PATH}/../plugins"
                "$<TARGET_FILE_DIR:MusicPlayer>/plugins")
        endif()
    endif()
endif()
