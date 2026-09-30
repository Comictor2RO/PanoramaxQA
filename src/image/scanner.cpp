#include "image/scanner.hpp"
#include "image/exif_parser.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace fs = std::filesystem;

const std::unordered_set<std::string> supported_extensions{
    ".jpg",
    ".jpeg",
    ".png",
    ".tif",
    ".tiff",
    ".webp"
};

std::vector<ImageMetadata> scanImage(const std::string& input_directory){
    fs::path input_path = input_directory;
    std::vector<fs::path> image_paths;
    
    for(const auto& entry : fs::directory_iterator(input_path)){
        if(!entry.is_regular_file()){
            continue;
        }

        std::string extension = entry.path().extension().string();

        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c){
            return static_cast<char>(std::tolower(c));
        });

        if(supported_extensions.find(extension) != supported_extensions.end())
            image_paths.push_back(entry.path());
    }

    std::sort(image_paths.begin(), image_paths.end());

    std::vector<ImageMetadata> images;

    for(const auto& image_path : image_paths){
        images.push_back(parseExif(image_path.string()));
    }

    return images;
}