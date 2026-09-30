#include "image/exif_parser.hpp"

namespace {
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
        const auto pitch = xmp_data.findKey(Exiv2::XmpKey("Xmp.Camera.Pitch"));
        if(pitch != xmp_data.end()){
            metadata.pitch = pitch->toFloat();
        }

        const auto roll = xmp_data.findKey(Exiv2::XmpKey("Xmp.Camera.Roll"));
        if(roll != xmp_data.end()){
            metadata.roll = roll->toFloat();
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

        parseXMP(xmpData, metadata);

        auto latitude = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude"));
        auto latitude_ref = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLatitudeRef"));
        auto longitude = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLongitude"));
        auto longitude_ref = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLongitudeRef"));

        if(latitude == exifData.end() || latitude_ref == exifData.end() || longitude == exifData.end() || longitude_ref == exifData.end()) {
            return metadata;
        }

        const auto latitude_value = read_coordinate(*latitude);
        const auto longitude_value = read_coordinate(*longitude);

        if(!latitude_value || !longitude_value) {
            return metadata;
        }

        metadata.latitude = latitude_value.value();
        metadata.longitude = longitude_value.value();

        const std::string lat_ref = latitude_ref->toString();
        const std::string lon_ref = longitude_ref->toString();

        if(lat_ref == "S") {
            metadata.latitude = -metadata.latitude;
        }
        else if(lat_ref != "N") {
            return metadata;
        }

        if(lon_ref == "W") {
            metadata.longitude = -metadata.longitude;
        }
        else if(lon_ref != "E") {
            return metadata;
        }

        if (std::abs(metadata.latitude) > 90.0 || std::abs(metadata.longitude) > 180.0)
            return metadata;

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
        bool has_timestamp = false;
        if(timestamp != exifData.end()){
            std::tm calendar_time{};
            std::istringstream timestamp_stream(timestamp->toString());

            timestamp_stream >> std::get_time(&calendar_time, "%Y:%m:%d %H:%M:%S");

            if(!timestamp_stream.fail()){
                const std::time_t time_value = std::mktime(&calendar_time);

                if(time_value != static_cast<std::time_t>(-1)){
                    metadata.timestamp = std::chrono::system_clock::from_time_t(time_value);

                    has_timestamp = true;
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

        metadata.is_valid = has_timestamp;
    }
    catch(const Exiv2::Error&) {
        metadata.is_valid = false;
    }

    return metadata;
}

