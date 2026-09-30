cmake_minimum_required(VERSION 3.20)

project(panoramax_qa)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED)
find_package(exiv2 CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(panoramax_qa
    src/main.cpp
    src/cli/args.cpp
    src/image/exif_parser.cpp
)
target_link_libraries(panoramax_qa
    PRIVATE
        ${OpenCV_LIBS}
        exiv2
        CLI11::CLI11
        nlohmann_json::nlohmann_json
)

target_include_directories(panoramax_qa
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
)