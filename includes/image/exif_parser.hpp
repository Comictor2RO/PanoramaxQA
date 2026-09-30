#pragma once

#include "image/metadata.hpp"
#include <exiv2/exiv2.hpp>
#include <optional>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <sstream>

ImageMetadata parseExif(const std::string& image_path);