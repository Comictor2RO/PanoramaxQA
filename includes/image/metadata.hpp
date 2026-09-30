#pragma once

#include <string>
#include <chrono>
#include <cstdint>

struct ImageMetadata{
    std::string file_path;
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double gps_accuracy = 0.0;
    double heading = 0.0;
    double pitch = 0.0;
    double roll = 0.0;

    std::chrono::system_clock::time_point timestamp;

    std::uint32_t width = 0;
    std::uint32_t height = 0;

    bool is_valid = false;
};