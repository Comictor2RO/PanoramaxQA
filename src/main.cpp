#include <iostream>
#include <exception>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include "cli/args.hpp"
#include "image/scanner.hpp"

const char* aiVerdictToString(AiVerdict verdict) {
    switch (verdict) {
        case AiVerdict::confirmed:
            return "confirmed";
        case AiVerdict::likely:
            return "likely";
        case AiVerdict::not_detected:
            return "not_detected";
        case AiVerdict::unknown:
            return "unknown";
    }

    return "unknown";
}
/*
int main(){
    std::string img_path = "./test_folder/2.jpg";
    cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR);

    if(img.empty()){
        std::cout << "Couldnt read the image";
        return 1;
    }

    cv::imshow("Display window", img);
    int k = cv::waitKey(0);

    if(k == 's'){
        cv::imwrite("idk", img);
    }

    return 0;
}
*/


int main(int argc, char** argv) {
    try {
        const CliArgs args = parseArgs(argc, argv);
        const std::vector<ImageMetadata> images =
            scanImage(args.input_directory);

        if (!args.quiet) {
            std::cout << "Images found: " << images.size() << '\n';
        }

        if (args.verbose) {
            for (const auto& image : images) {
                std::cout << image.file_path
                          << " | valid: " << std::boolalpha
                          << image.is_valid
                          << " | gps_valid: "
                          << image.has_valid_gps
                          << " | timestamp: "
                          << image.has_timestamp
                          << " | ai_verdict: "
                          << aiVerdictToString(image.ai_verdict)
                          << " | ai_confidence: "
                          << image.ai_confidence
                          << " | latitude: " << image.latitude
                          << " | longitude: " << image.longitude
                          << " | altitude: " << image.altitude
                          << " | heading: " << image.heading
                          << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}