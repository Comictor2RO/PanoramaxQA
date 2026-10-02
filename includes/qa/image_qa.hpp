#pragma once

#include "image/metadata.hpp"

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <string>
#include <vector>
#include <algorithm>

struct ImageQA{
    double sharpness = 0.0;
    double brightness = 0.0;
    double blur_score = 0.0;
    double gps_score = 0.0;

    bool is_blur_acceptable = false;
    bool is_brightness_acceptable = false;
    bool is_gps_acceptable = false;
    bool passed = false;
    bool is_ai_acceptable = true;

    AiVerdict ai_verdict = AiVerdict::unknown;
    std::vector<std::string> issues; 
};

ImageQA computeQA(const ImageMetadata& metadata, const cv::Mat& image, double blur_threshold, double brightness_threshold);




