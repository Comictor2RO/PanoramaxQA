#include "image/exif_parser.hpp"
#include "ai/c2pa_detector.hpp"

#include <algorithm>
#include <array>
#include <cctype>

namespace {
    std::string lowercase(std::string value) {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );

        return value;
    }

    void detectAiMetadata(
        const Exiv2::ExifData& exif_data,
        const Exiv2::XmpData& xmp_data,
        ImageMetadata& metadata
    ) {
        try {
            std::vector<std::string> software_values;

            const auto software = exif_data.findKey(
                Exiv2::ExifKey("Exif.Image.Software")
            );

            if (software != exif_data.end()) {
                software_values.push_back(software->toString());
            }

            const auto creator_tool = xmp_data.findKey(
                Exiv2::XmpKey("Xmp.xmp.CreatorTool")
            );

            if (creator_tool != xmp_data.end()) {
                software_values.push_back(creator_tool->toString());
            }

            constexpr std::array ai_markers{
                "midjourney",
                "stable diffusion",
                "dall-e",
                "adobe firefly",
                "generative fill",
                "comfyui",
                "automatic1111",
                "leonardo.ai",
                "ideogram",
                "byteplus_modelark"
            };

            for (const auto& original_value : software_values) {
                const std::string value = lowercase(original_value);

                for (const auto marker : ai_markers) {
                    if (value.find(marker) != std::string::npos) {
                        metadata.ai_verdict = AiVerdict::likely;
                        metadata.ai_confidence = 0.9;
                        metadata.ai_indicators.push_back(original_value);
                        break;
                    }
                }
            }

            if (metadata.ai_verdict == AiVerdict::likely) {
                return;
            }

            const std::array camera_keys{
                "Exif.Image.Make",
                "Exif.Image.Model",
                "Exif.Photo.ExposureTime",
                "Exif.Photo.FNumber",
                "Exif.Photo.LensModel"
            };

            std::size_t camera_field_count = 0;

            for (const auto key : camera_keys) {
                if (exif_data.findKey(Exiv2::ExifKey(key)) != exif_data.end()) {
                    ++camera_field_count;
                }
            }

            if (camera_field_count >= 2) {
                metadata.ai_verdict = AiVerdict::not_detected;
                metadata.ai_confidence = 0.6;
            }
        } catch (const Exiv2::Error&) {
            metadata.ai_verdict = AiVerdict::unknown;
            metadata.ai_confidence = 0.0;
        }
    }

    // Helper function to read GPS coordinates from Exif data
    std::optional<double> read_coordinate(const auto& datum){
        if(datum.count() < 3)
            return std::nullopt;

        const auto degrees = datum.value().toRational(0);
        const auto minutes = datum.value().toRational(1);
        const auto seconds = datum.value().toRational(2);

        if(degrees.second == 0 || minutes.second == 0 || seconds.second == 0)
            return std::nullopt;

        const double degree_value = static_cast<double>(degrees.first) / static_cast<double>(degrees.second);
        const double minute_value = static_cast<double>(minutes.first) / static_cast<double>(minutes.second);
        const double second_value = static_cast<double>(seconds.first) / static_cast<double>(seconds.second);
    
        if(minute_value < 0 || minute_value >= 60 || second_value < 0 || second_value >= 60)
            return std::nullopt;

        return degree_value + (minute_value / 60.0) + (second_value / 3600.0);
        
    }

    // Helper funciton to get pitch and roll from XMP Data
    void parseXMP(const Exiv2::XmpData& xmp_data, ImageMetadata& metadata){
        try{
            const auto pitch = xmp_data.findKey(Exiv2::XmpKey("Xmp.Camera.Pitch"));
            if(pitch != xmp_data.end()){
                metadata.pitch = pitch->toFloat();
            }

            const auto roll = xmp_data.findKey(Exiv2::XmpKey("Xmp.Camera.Roll"));
            if(roll != xmp_data.end()){
                metadata.roll = roll->toFloat();
            }
        }
        catch (const Exiv2::Error&){

        }
    }

}

// Function to parse Exif metadata from an image file
ImageMetadata parseExif(const std::string& image_path){
    ImageMetadata metadata;
    metadata.file_path = image_path;
    
    try {
        auto image = Exiv2::ImageFactory::open(image_path);

        if(image.get() == nullptr)
            return metadata;

        image->readMetadata();
        
        const Exiv2::ExifData& exifData = image->exifData();
        const Exiv2::XmpData& xmpData = image->xmpData();

        detectAiMetadata(exifData, xmpData, metadata);
        detectC2paMetadata(image_path, metadata);
        parseXMP(xmpData, metadata);

        auto latitude = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude"));
        auto latitude_ref = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLatitudeRef"));
        auto longitude = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLongitude"));
        auto longitude_ref = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLongitudeRef"));

        const bool has_gps_keys = latitude != exifData.end() && latitude_ref != exifData.end() && longitude != exifData.end() && longitude_ref != exifData.end();

        if (has_gps_keys) {
            const auto latitude_value = read_coordinate(*latitude);
            const auto longitude_value = read_coordinate(*longitude);

            if (latitude_value && longitude_value) {
                metadata.latitude = latitude_value.value();
                metadata.longitude = longitude_value.value();

                const std::string lat_ref = latitude_ref->toString();
                const std::string lon_ref = longitude_ref->toString();

                const bool valid_latitude_ref = lat_ref == "N" || lat_ref == "North" || lat_ref == "S" || lat_ref == "South";

                const bool valid_longitude_ref = lon_ref == "E" || lon_ref == "East" || lon_ref == "W" || lon_ref == "West";

                if (lat_ref == "S" || lat_ref == "South") {
                    metadata.latitude = -metadata.latitude;
                }

                if (lon_ref == "W" || lon_ref == "West") {
                    metadata.longitude = -metadata.longitude;
                }

                metadata.has_valid_gps = valid_latitude_ref && valid_longitude_ref && std::abs(metadata.latitude) <= 90.0 && std::abs(metadata.longitude) <= 180.0;
            }
        }

        const auto altitude = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude"));
        if(altitude != exifData.end()){
            metadata.altitude = altitude->toFloat();
            const auto altitude_ref = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef"));

            if(altitude_ref != exifData.end() && altitude_ref->toLong() == 1){
                metadata.altitude = -metadata.altitude;
            }
        }

        const auto gps_dop = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSDOP"));
        if(gps_dop != exifData.end()){
            metadata.gps_accuracy = gps_dop->toFloat();
        }

        auto heading = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSImgDirection"));
        if(heading != exifData.end()){
            metadata.heading = heading->toFloat();
        }else{
            heading = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSTrack"));
            if(heading != exifData.end()){
                metadata.heading = heading->toFloat();
            }
        }

        const auto timestamp = exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
        if(timestamp != exifData.end()){
            std::tm calendar_time{};
            std::istringstream timestamp_stream(timestamp->toString());

            timestamp_stream >> std::get_time(&calendar_time, "%Y:%m:%d %H:%M:%S");

            if(!timestamp_stream.fail()){
                const std::time_t time_value = std::mktime(&calendar_time);

                if(time_value != static_cast<std::time_t>(-1)){
                    metadata.timestamp = std::chrono::system_clock::from_time_t(time_value);

                    metadata.has_timestamp = true;
                }
            }
        }

        const auto width = exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
        if(width != exifData.end()){
            metadata.width = static_cast<std::uint32_t>(width->toLong());
        }

        const auto height = exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
        if(height != exifData.end()){
            metadata.height = static_cast<std::uint32_t>(height->toLong());
        }

        metadata.is_valid =
            metadata.has_valid_gps && metadata.has_timestamp;
    }
    catch(const Exiv2::Error& error) {
        std::cerr << "Exiv2 error for " << image_path << ": " << error.what() << '\n';
        metadata.is_valid = false;
    }

    return metadata;
}

