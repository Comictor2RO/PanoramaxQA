#include "cli/args.hpp"

namespace{
    // Expands a path that starts with ~ to the user's home directory.
    std::string expandUserPath(const std::string& path){
        if(path != "~" && (path.size() < 2 || path.compare(0, 2, "~/") != 0))
            return path;

        const char *home = std::getenv("HOME");
            
        if(home == nullptr)
            throw std::runtime_error("HOME environment variable is not set");
            
        if(path == "~")
            return home;

        return (std::filesystem::path(home) / path.substr(2)).string();
    }
}


// Parses command line arguments and returns a CliArgs struct with the parsed values.
CliArgs parseArgs(int argc, char **argv) {
    CliArgs args;
    constexpr const char *version = "0.0.1";
    
    CLI::App app{"Panoramax QA"};
    app.set_version_flag("--version", version);

    app.add_option("-i,--input", args.input_directory, "Input Folder")
        ->required()
        ->check(CLI::ExistingDirectory);

    app.add_option("-B,--blur", args.threshold_blur, "Blur Threshold")
        ->capture_default_str()
        ->check(CLI::PositiveNumber);

    app.add_option("-b,--brightness", args.threshold_brightness, "Brightness Threshold")
        ->capture_default_str()
        ->check(CLI::Range(0.0, 255.0));

    app.add_option("-j,--jump", args.max_gps_jump_meters, "Max GPS Jump (meters)")
        ->capture_default_str()
        ->check(CLI::PositiveNumber);

    app.add_option("-r,--report", args.report_path, "Report Path")
        ->capture_default_str();

    app.add_flag("--upload", args.do_upload, "Upload Accepted Images");

    app.add_flag("--quiet", args.quiet, "Quiet Output");

    app.add_flag("--verbose", args.verbose, "Verbose Output");
    
    app.add_option("-t,--token", args.token_path, "Token Path")
        ->capture_default_str();

    app.add_option("-a,--api", args.api_base_url, "API Base URL")
        ->capture_default_str();

    try {
        app.parse(argc, argv);

        if(args.verbose && args.quiet){
            throw CLI::ValidationError("Cannot use both --verbose and --quiet flags at the same time.");
        }

        if(args.do_upload){
            args.token_path = expandUserPath(args.token_path);

            std::ifstream token_file(args.token_path);
            if(!token_file){
                throw CLI::ValidationError("Token file cannot be opened: " + args.token_path);
            }
        }
    } catch (const CLI::ParseError &error) {
        std::exit(app.exit(error));
    }

    return args;
}