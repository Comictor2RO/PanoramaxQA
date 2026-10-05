#pragma once

#include "image/metadata.hpp"

#include <string>
#include <vector>

struct SequenceIssue{
    std::string type;
    std::string first_image;
    std::string second_image;
    double value = 0.0;
};

struct SequenceQA{
    bool passed = true;
    std::vector<SequenceIssue> issues;
};

SequenceQA analyzeSequence(const std::vector<ImageMetadata>& images, double max_gps_jump_meters, double max_heading_difference_degrees);