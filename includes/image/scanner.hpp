#pragma once

#include "image/metadata.hpp"

#include <filesystem>
#include <string>
#include <vector>

std::vector<ImageMetadata> scanImage(const std::string& input_directory);