# Panoramax QA

Panoramax QA is a C++23 application that scans images, reads EXIF/XMP and C2PA metadata, and prepares data for quality checks and report generation.

## Current status

Implemented:

- CLI argument parsing with CLI11;
- scanning a folder without subdirectories;
- filtering `.jpg`, `.jpeg`, `.png`, `.tif`, `.tiff`, and `.webp` extensions;
- deterministic image sorting;
- parsing GPS data from EXIF;
- converting coordinates from degrees, minutes, and seconds to decimal degrees;
- reading altitude, GPSDOP, heading, timestamp, and dimensions;
- optionally reading pitch/roll values from XMP;
- detecting AI indicators in EXIF/XMP;
- reading C2PA manifests;
- displaying the AI verdict in verbose mode;
- calculating brightness and sharpness for each image;
- calculating a normalized blur score;
- calculating the GPS score;
- generating issues and a per-image QA verdict.

Not yet implemented:

- calculating pixel density;
- fully integrating QA results into the final report;
- sequence analysis;
- JSON/CSV reporting;
- complete unit tests;
- uploading to the Panoramax API.

## Dependencies

### CLI11

CLI11 is used to parse command-line arguments:

- `--input` / `-i` for the input folder;
- `--blur` / `-B` for the blur threshold;
- `--brightness` / `-b` for the brightness threshold;
- `--jump` / `-j` for the maximum GPS jump;
- `--report` / `-r` for the report path;
- `--upload` / `-u` to enable uploading;
- `--token` / `-t` for the token;
- `--api` / `-a` for the API URL;
- `--verbose`, `--quiet`, and `--version`.

CLI11 validates the input folder, numeric values, and parsing errors.

### Exiv2

Exiv2 is used for EXIF and XMP metadata:

- GPS coordinates;
- GPS references `N`, `S`, `E`, `W`;
- altitude;
- GPSDOP;
- heading;
- EXIF timestamp;
- width and height;
- some XMP values.

Exiv2 errors must not stop processing all images. An image with missing or invalid metadata is kept in the results with default values and `is_valid = false`.

### C2PA

C2PA is used to read and validate provenance manifests in images. The project uses the C++ `c2pa-cpp` library, downloaded through CMake `FetchContent`.

The C2PA detector:

1. attempts to open the image with `c2pa::Reader::from_asset()`;
2. ignores images without a C2PA manifest;
3. reads the manifest as JSON;
4. looks for providers and actions that indicate AI generation;
5. adds the indicators found to `ImageMetadata`;
6. combines the result with the EXIF/XMP fallback.

The C2PA manifest is preferable to a simple text search because the library can verify the binding and validate the manifest.

### OpenCV

OpenCV is used for image analysis:

- loading images;
- calculating brightness;
- calculating sharpness using the variance of Laplacian;
- calculating the normalized blur score;
- other future QA metrics.

### nlohmann/json

`nlohmann/json` is used to read the JSON returned by C2PA and will later be used to generate the JSON report.

## Per-image QA

The `computeQA()` function combines the OpenCV image with `ImageMetadata` and produces an `ImageQA` result.

### Brightness

Brightness is the mean of the three BGR channels and is compared with the `--brightness` threshold.
Images below the threshold receive the `too_dark` issue.

### Blur and sharpness

The image is converted to grayscale, then the variance of Laplacian is calculated:

- low variance indicates a blurry image;
- high variance indicates a sharper image.

The value is normalized to a `blur_score` between `0` and `1` and compared with the `--blur` threshold. Images below the threshold receive the `too_blurry` issue.

### GPS score

`gps_score` is `1.0` when `ImageMetadata::has_valid_gps` is true and `0.0` otherwise. Missing valid GPS data adds the `invalid_gps` issue.

### AI verdict in QA

The AI verdict is taken from `ImageMetadata`:

- `confirmed` adds `ai_generated` and rejects the image;
- `likely` adds `likely_ai_generated` and rejects the image;
- `not_detected` adds no issue and passes the AI check;
- `unknown` adds `ai_status_unknown`, but does not automatically reject the image.

The final `passed` verdict is true only when blur, brightness, GPS, and the AI policy are acceptable.

## Detecting AI-generated images

The current detection is based on metadata and provenance indicators. It is not a visual detector and cannot prove that an image is real simply because it has no AI indicators.

### EXIF/XMP indicators

The detector looks for values in:

- `Exif.Image.Software`;
- `Xmp.xmp.CreatorTool`.

The markers searched for include:

- Midjourney;
- Stable Diffusion;
- DALL-E;
- Adobe Firefly;
- Generative Fill;
- ComfyUI;
- Automatic1111;
- Leonardo AI;
- Ideogram;
- BytePlus ModelArk.

A marker found in EXIF or XMP produces the `likely` verdict.

### C2PA indicators

For C2PA manifests, the detector searches for values such as:

- the software provider;
- `BytePlus_ModelArk`;
- creation or generation actions;
- text indicating generation.

If an AI provider or action exists, the indicator is kept in `ai_indicators`.

C2PA is stronger than EXIF/XMP when the manifest is valid and its signature can be verified. In the application, a manifest without validation errors can produce the `confirmed` verdict, while a manifest with validation errors produces `likely`.

## AI verdicts

The verdict is stored in:

```cpp
AiVerdict ai_verdict;
```

and can have one of the following values.

### `confirmed`

The C2PA manifest indicates AI generation and reports no validation errors.

This is the strongest verdict currently available in the implementation. It depends on validation performed by the C2PA library.

### `likely`

Explicit AI indicators were found in EXIF, XMP, or in a C2PA manifest with validation problems.

Examples:

- `Software = Stable Diffusion`;
- `CreatorTool = Midjourney`;
- a known C2PA provider;
- a C2PA generation action with incomplete validation.

This verdict is likely, but metadata can be modified or forged.

### `not_detected`

No AI indicator was found, and the image contains at least two coherent camera fields, for example:

- `Make`;
- `Model`;
- `ExposureTime`;
- `FNumber`;
- `LensModel`.

This verdict only means that AI generation was not detected. It is not absolute proof that the image is real.

### `unknown`

There is not enough information to reach a conclusion:

- missing metadata;
- an image exported without EXIF/XMP/C2PA;
- an error while reading the manifest;
- an image without coherent camera fields and without AI indicators.

An AI-generated image whose metadata has been removed will generally receive the `unknown` verdict.

## Confidence and indicators

The result is stored in `ImageMetadata`:

```cpp
AiVerdict ai_verdict = AiVerdict::unknown;
double ai_confidence = 0.0;
std::vector<std::string> ai_indicators;
```

The current values are indicative:

- `1.0` for `confirmed`;
- `0.9` for `likely`;
- `0.6` for `not_detected`;
- `0.0` for `unknown`.

These values are not calibrated statistical probabilities.

## Build

```bash
cmake -S . -B build
cmake --build build --parallel
```

C2PA is downloaded through `FetchContent`, so the first configuration requires internet access.

## Usage

Display help:

```bash
./build/panoramax_qa --help
```

Display the version:

```bash
./build/panoramax_qa --version
```

Process a folder:

```bash
./build/panoramax_qa \
    --input ./test_folder \
    --verbose
```

Example output:

```text
Images found: 1
./test_folder/image.jpg | valid: true | ai_verdict: likely | ai_confidence: 0.9
```

To inspect an image's metadata:

```bash
exiv2 -pa image.jpg
```

## Limitations

- Missing metadata does not prove that an image was not AI-generated.
- EXIF and XMP can be deleted or modified.
- A provider may use a name that is not currently in the list.
- C2PA detection depends on the manifest, signature, and format support.
- CBOR is processed through C2PA when it is part of a supported manifest; Exiv2 is not used to read C2PA manifests.
- `pitch` and `roll` do not have universal EXIF tags.
