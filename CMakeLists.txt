cmake_minimum_required(VERSION 3.20)

project(panoramax_qa)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    c2pa_cpp
    GIT_REPOSITORY https://github.com/contentauth/c2pa-cpp.git
    GIT_TAG v0.26.13
)

FetchContent_MakeAvailable(c2pa_cpp)

find_package(OpenCV REQUIRED)
find_package(exiv2 CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(panoramax_qa
    src/main.cpp
    src/cli/args.cpp
    src/image/exif_parser.cpp
    src/image/scanner.cpp
    src/ai/c2pa_detector.cpp
    src/qa/image_qa.cpp
)
target_link_libraries(panoramax_qa
    PRIVATE
        ${OpenCV_LIBS}
        exiv2
        CLI11::CLI11
        nlohmann_json::nlohmann_json
        c2pa_cpp
)

target_include_directories(panoramax_qa
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
)