#include "ai/c2pa_detector.hpp"

#include <cctype>
#include <filesystem>
#include <memory>
#include <string>

#include <c2pa.hpp>
#include <nlohmann/json.hpp>

namespace {
    using json = nlohmann::json;

    std::string lowercase(std::string value) {
        for (char& character : value) {
            character = static_cast<char>(
                std::tolower(static_cast<unsigned char>(character))
            );
        }
        return value;
    }

    void collectStrings(const json& value, std::vector<std::string>& result) {
        if (value.is_string()) {
            result.push_back(value.get<std::string>());
            return;
        }

        if (value.is_array()) {
            for (const auto& item : value) {
                collectStrings(item, result);
            }
            return;
        }

        if (value.is_object()) {
            for (const auto& [key, item] : value.items()) {
                result.push_back(key);
                collectStrings(item, result);
            }
        }
    }

    bool containsString(const std::vector<std::string>& values, const std::string& marker) {
        for (const auto& value : values) {
            if (lowercase(value).find(marker) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    std::string findProvider(const std::vector<std::string>& values) {
        struct Provider {
            const char* name;
            const char* marker;
        };

        constexpr Provider providers[] = {
            {"BytePlus ModelArk", "byteplus_modelark"},
            {"BytePlus ModelArk", "modelark"},
            {"Midjourney", "midjourney"},
            {"OpenAI", "dall-e"},
            {"OpenAI", "openai"},
            {"Stability AI", "stable diffusion"},
            {"Stability AI", "stability ai"},
            {"Adobe Firefly", "adobe firefly"},
            {"Adobe Firefly", "firefly"},
            {"ComfyUI", "comfyui"},
            {"Automatic1111", "automatic1111"},
            {"Leonardo AI", "leonardo.ai"},
            {"Ideogram", "ideogram"}
        };

        for (const auto& value : values) {
            const std::string normalized = lowercase(value);
            for (const auto& provider : providers) {
                if (normalized.find(provider.marker) != std::string::npos) {
                    return provider.name;
                }
            }
        }

        return {};
    }

    bool hasValidationErrors(const json& manifest_store) {
        if (!manifest_store.is_object() ||
            !manifest_store.contains("validation_status")) {
            return false;
        }

        const auto& status = manifest_store["validation_status"];
        return !status.is_null() &&
               (!status.is_array() || !status.empty());
    }
}

void detectC2paMetadata(const std::string& image_path, ImageMetadata& metadata) {
    try {
        auto context = std::make_shared<c2pa::Context>();
        auto reader = c2pa::Reader::from_asset(
            context,
            std::filesystem::path(image_path)
        );

        if (!reader.has_value()) {
            return;
        }

        const json manifest_store = json::parse(reader->json());
        std::vector<std::string> values;
        collectStrings(manifest_store, values);

        const std::string provider = findProvider(values);
        const bool has_ai_action =
            containsString(values, "c2pa.created") ||
            containsString(values, "generative") ||
            containsString(values, "generated");

        if (provider.empty() && !has_ai_action) {
            return;
        }

        if (!provider.empty()) {
            metadata.ai_indicators.push_back(
                "C2PA provider: " + provider
            );
        }

        if (has_ai_action) {
            metadata.ai_indicators.push_back("C2PA AI creation action");
        }

        metadata.ai_verdict =
            hasValidationErrors(manifest_store)
                ? AiVerdict::likely
                : AiVerdict::confirmed;

        metadata.ai_confidence =
            metadata.ai_verdict == AiVerdict::confirmed
                ? 1.0
                : 0.9;
    } catch (const c2pa::C2paException&) {
        // C2PA este opțional; detectorul EXIF/XMP trebuie să continue.
    } catch (const json::parse_error&) {
        // Un manifest imposibil de interpretat nu trebuie să invalideze imaginea.
    }
}
