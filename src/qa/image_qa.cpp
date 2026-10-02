#include "qa/image_qa.hpp"

ImageQA computeQA(const ImageMetadata& metadata, const cv::Mat& image, double blur_threshold, double brightness_threshold){
    ImageQA result;
    constexpr double reference_sharpness = 500.0;

    if(image.empty()){
        result.issues.push_back("image_unreadable");
        result.passed = false;
        return result;
    }

    cv::Scalar mean_bgr = cv::mean(image);
    result.brightness = (mean_bgr[0] + mean_bgr[1] + mean_bgr[2]) / 3;

    result.is_brightness_acceptable = result.brightness >= brightness_threshold;

    if(!result.is_brightness_acceptable)
        result.issues.push_back("too_dark");

    // Converts the image in grayscale
    cv::Mat grayscale;
    cv::cvtColor(image, grayscale, cv::COLOR_BGR2GRAY);

    // Calcute the Laplacian
    cv::Mat laplacian;
    cv::Laplacian(grayscale, laplacian, CV_64F);

    // Calculate the variance of Laplace(varince is low -> blurry image, variance is high -> clear image)
    cv::Scalar mean;
    cv::Scalar standard_deviation;

    cv::meanStdDev(laplacian, mean, standard_deviation);
    result.sharpness = standard_deviation[0] * standard_deviation[0];

    result.blur_score = std::min(1.0, result.sharpness / reference_sharpness);
    result.is_blur_acceptable = result.blur_score >= blur_threshold;

    if(!result.is_blur_acceptable)
        result.issues.push_back("too_blurry");

    if(metadata.has_valid_gps){
        result.gps_score = 1.0;
        result.is_gps_acceptable = true;
    }
    else{
        result.gps_score = 0.0;
        result.is_gps_acceptable = false;
        result.issues.push_back("invalid_gps");
    }

    result.ai_verdict = metadata.ai_verdict;
    switch(result.ai_verdict){
        case AiVerdict::confirmed:
            result.is_ai_acceptable = false;
            result.issues.push_back("ai_generated");
            break;

        case AiVerdict::likely:
            result.is_ai_acceptable = false;
            result.issues.push_back("likely_ai_generated");
            break;

        case AiVerdict::not_detected:
            result.is_ai_acceptable = true;
            break;

        case AiVerdict::unknown:
            result.is_ai_acceptable = true;
            result.issues.push_back("ai_status_unknown");
            break;
    }

    result.passed = result.is_ai_acceptable && result.is_blur_acceptable && result.is_brightness_acceptable && result.is_gps_acceptable;
    return result;
}
