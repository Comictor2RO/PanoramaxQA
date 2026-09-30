#include <iostream>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include "cli/args.hpp"


int main(int argc, char** argv) {
    CliArgs args = parseArgs(argc, argv);



    return 0;
}