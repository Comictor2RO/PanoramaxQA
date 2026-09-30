#pragma once

#include <string>
#include <CLI/CLI.hpp>
#include <filesystem>
#include <stdexcept>
#include <cstdlib>
#include <fstream>

struct CliArgs {
    std::string input_directory;
    double threshold_blur = 0.25;
    double threshold_brightness = 30.0;
    double max_gps_jump_meters = 50.0;
    std::string report_path = "report.json";
    bool do_upload = false;
    bool verbose = false;
    bool quiet = false;
    std::string token_path = "~/.panoramax/token";
    std::string api_base_url = "https://panoramax.ign.fr/api";
};

CliArgs parseArgs(int argc, char **argv);