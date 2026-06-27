cmake_minimum_required(VERSION 3.16)
project(algoviz VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(algoviz
    src/main.cpp
    src/MainWindow.cpp
    src/SortView.cpp
    src/PathView.cpp
    src/HashView.cpp
    src/DPView.cpp
    src/PlaybackBar.cpp
    include/MainWindow.h
    include/SortView.h
    include/PathView.h
    include/HashView.h
    include/DPView.h
    include/PlaybackBar.h
)

target_include_directories(algoviz PRIVATE include)
target_link_libraries(algoviz PRIVATE Qt6::Widgets)

install(TARGETS algoviz RUNTIME DESTINATION bin)
