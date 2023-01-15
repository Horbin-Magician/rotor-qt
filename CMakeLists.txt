cmake_minimum_required(VERSION 3.5)

project(Rotor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    Utils/powerboot.h
    Utils/UAC.h
    Utils/hotkey.h

    Models/setting_model.h Models/setting_model.cpp
    GUI/w_setting.h GUI/w_setting.cpp
    rotor.h rotor.cpp
    main.cpp
    resources.qrc
    appicon.rc

    Modules/i_module.h

    Modules/Searcher/Components/search_result_list.h Modules/Searcher/Components/search_result_list.cpp
    Modules/Searcher/searcher.h Modules/Searcher/searcher.cpp
    Modules/Searcher/FileData/file_data.h Modules/Searcher/FileData/file_data.cpp
    Modules/Searcher/FileData/volume.h Modules/Searcher/FileData/volume.cpp

    Modules/ScreenShotter/screen_shotter.h Modules/ScreenShotter/screen_shotter.cpp
    Modules/ScreenShotter/Components/amplifier.h Modules/ScreenShotter/Components/amplifier.cpp
    Modules/ScreenShotter/Components/shotter_window.h Modules/ScreenShotter/Components/shotter_window.cpp
    Modules/ScreenShotter/Components/toolbar.h Modules/ScreenShotter/Components/toolbar.cpp
)

# add icon
#set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Rotor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Rotor APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Rotor SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Rotor
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Rotor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(Rotor PRIVATE Dwmapi.lib)

set_target_properties(Rotor PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Rotor)
endif()
