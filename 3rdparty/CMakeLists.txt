cmake_minimum_required(VERSION 3.5)

project(video_concatenater_3rdparty_libs VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(qt_collapsible_section STATIC
    qt_collapsible_section/Section.cpp
)

target_link_libraries(qt_collapsible_section PRIVATE
    Qt6::Widgets
)

target_include_directories(qt_collapsible_section PUBLIC
    qt_collapsible_section
)