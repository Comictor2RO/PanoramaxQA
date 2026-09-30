#include <iostream>
#include <exception>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include "cli/args.hpp"
#include "image/scanner.hpp"


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