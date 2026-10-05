# Panoramax QA — Milestones

## Objective

Panoramax QA is a C++ application that analyzes images, extracts EXIF/XMP metadata, calculates quality metrics, detects sequence-level issues, and generates a JSON report. At a later stage, the application will be able to upload accepted images to the Panoramax API.

## Implemented

### Initial structure

- The project uses CMake.
- The C++ compiler is configured with the C++23 standard.
- Separate directories exist for source code and headers.
- The project executable is named `panoramax_qa`.
- The build works in the separate `build/` folder.

Current relevant structure:

```text
panoramax_qa/
├── CMakeLists.txt
├── includes/
│   ├── cli/
│   │   └── args.hpp
│   ├── image/
│       ├── metadata.hpp
│       ├── exif_parser.hpp
│       └── scanner.hpp
│   └── ai/
│       └── c2pa_detector.hpp
│   └── qa/
│       └── image_qa.hpp
├── src/
│   ├── main.cpp
│   ├── cli/
│   │   └── args.cpp
│   └── image/
│       ├── exif_parser.cpp
│       └── scanner.cpp
│   └── ai/
│       └── c2pa_detector.cpp
│   └── qa/
│       └── image_qa.cpp
└── build/
```

### CMake

- CMake detects OpenCV.
- CMake detects nlohmann/json.
- CLI11 is included and linked through the `CLI11::CLI11` target.
- Exiv2 is declared as a dependency.
- C2PA is downloaded through `FetchContent` and linked through the `c2pa_cpp` target.
- `src/main.cpp`, `src/cli/args.cpp`, `src/image/exif_parser.cpp`, `src/image/scanner.cpp`, and `src/ai/c2pa_detector.cpp` are included in the executable.
- The `includes/` directory is available for including project headers.
- The project configures and compiles successfully.

Build commands:

```bash
cmake -S . -B build
cmake --build build
```

Run the executable with:

```bash
./build/panoramax_qa
```

### CLI parsing

The `CliArgs` structure is defined in `includes/cli/args.hpp`.

Existing fields:

- `input_directory` — the folder containing the images.
- `threshold_blur` — the blur threshold.
- `threshold_brightness` — the brightness threshold.
- `max_gps_jump_meters` — the maximum accepted GPS jump.
- `report_path` — the JSON report path.
- `do_upload` — enables or disables uploading.
- `verbose` — enables detailed output.
- `quiet` — suppresses normal output.
- `token_path` — the path to the Panoramax token.
- `api_base_url` — the base URL of the Panoramax API.

`parseArgs()` uses CLI11 and implements:

- the required `--input` / `-i` option;
- folder existence validation with `CLI::ExistingDirectory`;
- the `--blur` / `-B` option with `CLI::PositiveNumber`;
- the `--brightness` / `-b` option with a `0–255` range;
- the `--jump` / `-j` option with `CLI::PositiveNumber`;
- the `--report` / `-r` option;
- the `--upload` / `-u` flag;
- the `--verbose` flag;
- the `--quiet` flag;
- the `--version` flag;
- the `--token` / `-t` option;
- the `--api` / `-a` option;
- expansion of paths beginning with `~` for the token;
- validation that the token can be opened when uploading is enabled;
- rejection of simultaneous use of `--verbose` and `--quiet`;
- automatic display of help and parsing errors.

Example run:

```bash
./build/panoramax_qa \
    --input ./images \
    --blur 0.4 \
    --brightness 40 \
    --jump 100 \
    --report result.json
```

### Metadata model

The `ImageMetadata` structure is defined in `includes/image/metadata.hpp`.

The structure contains fields for:

- image path;
- latitude;
- longitude;
- altitude;
- GPS accuracy;
- heading;
- pitch;
- roll;
- timestamp;
- width and height;
- `is_valid` status;
- AI detection verdict;
- AI detection confidence;
- AI detection indicators.

### EXIF and XMP parsing

The following function is declared:

```cpp
ImageMetadata parseExif(const std::string& image_path);
```

The function:

- opens an image with Exiv2;
- reads metadata through `readMetadata()`;
- accesses `ExifData`;
- checks the essential GPS tags;
- converts GPS coordinates from degrees, minutes, and seconds to decimal degrees;
- applies the `N`, `S`, `E`, and `W` references;
- reads altitude and altitude reference;
- reads GPSDOP;
- reads the heading, falling back from `GPSImgDirection` to `GPSTrack`;
- parses the `DateTimeOriginal` timestamp;
- reads width and height from EXIF;
- handles Exiv2 errors without stopping processing of the remaining images;
- optionally reads pitch and roll from XMP;
- marks metadata as valid when GPS and timestamp data are valid.

XMP parsing is optional. An unknown XMP namespace, such as `Camera`, must not stop EXIF parsing.

The parser was tested with an image containing GPS, altitude, heading, and timestamp data. The result was:

```text
valid: true
latitude: 44.4268
longitude: 26.1025
altitude: 80
heading: 135
```

### Folder scanning

The following function is implemented:

```cpp
std::vector<ImageMetadata> scanImage(const std::string& input_directory);
```

The scanner:

- uses `std::filesystem::directory_iterator`;
- does not process subdirectories;
- accepts `.jpg`, `.jpeg`, `.png`, `.tif`, `.tiff`, and `.webp` extensions;
- treats extensions case-insensitively;
- sorts paths in a deterministic order;
- calls `parseExif()` for each image;
- returns the results in a `std::vector<ImageMetadata>`.

### Detecting AI-generated images

The C2PA detector is implemented in `src/ai/c2pa_detector.cpp`.

The detector:

- opens the C2PA manifest using `c2pa::Reader::from_asset()`;
- reads the manifest as JSON;
- looks for providers and actions that indicate AI generation;
- recognizes identifiers such as `BytePlus_ModelArk`, Midjourney, DALL-E, Stable Diffusion, Firefly, ComfyUI, and Ideogram;
- keeps the indicators in `ai_indicators`;
- combines the C2PA result with the EXIF/XMP fallback.

The available verdicts are:

- `confirmed` — a C2PA manifest with AI indicators and no validation errors;
- `likely` — an AI marker found in EXIF/XMP or a C2PA manifest with incomplete validation;
- `not_detected` — no AI marker exists and the image has coherent camera metadata;
- `unknown` — there is not enough metadata to reach a conclusion.

The verdicts are based on metadata and provenance. The absence of an AI marker does not prove that the image is real.

### Per-image QA

The following function is implemented:

```cpp
ImageQA computeQA(
    const ImageMetadata& metadata,
    const cv::Mat& image,
    double blur_threshold,
    double brightness_threshold
);
```

Per-image QA:

- checks whether the image can be loaded;
- calculates the mean brightness across the BGR channels;
- converts the image to grayscale;
- calculates sharpness using the variance of Laplacian;
- normalizes sharpness to a `blur_score` between `0` and `1`;
- checks GPS using `metadata.has_valid_gps`;
- takes the AI verdict from the metadata;
- adds issues such as `image_unreadable`, `too_dark`, `too_blurry`, `invalid_gps`, `ai_generated`, and `likely_ai_generated`;
- calculates the final `passed` verdict.

An image passes QA only if brightness, blur, GPS, and the AI policy are acceptable.

### Current integration in `main.cpp`

`main.cpp`:

- parses CLI arguments;
- calls the scanner for the provided folder;
- displays the number of images found;
- displays detailed metadata with `--verbose`;
- handles errors through `stderr` messages and a non-zero exit code.

## Not yet implemented

- Handling all XMP namespaces specific to camera manufacturers.
- Reading resolution from OpenCV as a fallback when EXIF does not contain dimensions.
- Calculating pixel density.
- Fully integrating `ImageQA` results into the main flow and report.
- Sequence analysis.
- Detecting GPS jumps.
- Checking heading consistency.
- Detecting duplicate images.
- Detecting faces with OpenCV and a Haar cascade.
- Generating the JSON report.
- Generating the CSV report.
- Panoramax HTTP client.
- Token authentication.
- Image uploading.
- Terminal progress bar and statistics.
- Unit tests.
- Final documentation and complete usage examples.

## Next steps

### [x] Milestone 1 — Completed: CLI

The CLI was implemented and tested for help, version, input, thresholds, uploading, token, verbose, and quiet modes.

1. Verify that `args.cpp` is included in `add_executable()`.
2. Compile the project.
3. Run `--help`.
4. Test the missing `--input` argument.
5. Test a nonexistent folder.
6. Test invalid values for blur, brightness, and GPS jump.
7. Test displaying parsed values in `main.cpp`.

Commands:

```bash
cmake -S . -B build
cmake --build build
./build/panoramax_qa --help
```

### [x] Milestone 2 — Completed: EXIF parsing for a single image

The parser was tested with images without GPS and with a test image whose GPS metadata was added using ExifTool.

1. Implement opening the image with `Exiv2::ImageFactory::open()`.
2. Call `readMetadata()`.
3. Read `ExifData`.
4. Check for GPS tags.
5. Implement conversion from degrees/minutes/seconds to decimal degrees.
6. Apply the correct sign for `N`, `S`, `E`, and `W`.
7. Read altitude and GPSDOP.
8. Read the heading using fallback between `GPSImgDirection` and `GPSTrack`.
9. Read the timestamp and dimensions.
10. Test images with complete metadata and images without GPS.

### [x] Milestone 3 — Completed: folder scanning

The scanner was tested on a folder containing JPEG and PNG images, including uppercase extensions.

1. Use `std::filesystem` to traverse the folder.
2. Accept only supported image extensions.
3. Sort images in a deterministic order.
4. Call `parseExif()` for each image.
5. Keep the results in a `std::vector<ImageMetadata>`.
6. Report the number of processed images and errors.

Example structure:

```cpp
std::vector<ImageMetadata> images;
```

### [x] Milestone 4 — Completed: per-image QA

Image loading and basic checks were implemented for one image. Pixel density and report integration remain for the next steps.

1. Create `includes/qa/image_qa.hpp`.
2. Define the `ImageQA` structure.
3. Implement image loading with OpenCV.
4. Calculate mean brightness.
5. Calculate the variance of Laplacian for sharpness.
6. Convert sharpness into a clearly defined blur score.
7. Calculate the GPS score.
8. Add issues for detected problems.
9. Connect the thresholds from `CliArgs` to the QA verdict.

### Milestone 5 — Sequence-level QA

1. Sort images by timestamp.
2. Compare consecutive images.
3. Implement the Haversine distance.
4. Detect GPS jumps.
5. Calculate the heading difference.
6. Detect inconsistent headings.
7. Detect possible duplicates.
8. Store the results in a `SequenceQA` structure.

### Milestone 6 — Face detection with OpenCV and Haar cascade

1. Add the OpenCV object-detection components required for face detection.
2. Load a configurable Haar cascade classifier.
3. Convert each image to grayscale and equalize the histogram when needed.
4. Detect faces using `cv::CascadeClassifier::detectMultiScale()`.
5. Store the number and bounding boxes of detected faces in the image QA result.
6. Add a configurable policy for images containing faces.
7. Add a clear QA issue when a face is detected.
8. Test detection on images with faces, without faces, and with different image sizes.

### Milestone 7 — JSON report

1. Create `includes/report/generator.hpp`.
2. Create `src/report/generator.cpp`.
3. Define the report structure.
4. Add the global summary.
5. Add the list of images and their issues.
6. Add sequence results.
7. Write the report using nlohmann/json.
8. Handle file-writing errors.

### Milestone 8 — Panoramax API connection

1. Create an HTTP client for the Panoramax API.
2. Create `includes/api/client.hpp`.
3. Create `src/api/client.cpp`.
4. Use `api_base_url` from `CliArgs` as the API base URL.
5. Read the authentication token from `token_path`.
6. Send the token using the authentication mechanism required by the Panoramax API.
7. Implement connection, timeout, and request error handling.
8. Validate HTTP status codes and report API errors clearly.
9. Add a configurable API endpoint for testing.
10. Ensure that API failures do not silently appear as successful requests.

### Milestone 9 — Image upload

1. Upload only images that passed the QA checks.
2. Build the multipart/form-data request expected by the Panoramax API.
3. Include the image file and the required metadata in the upload request.
4. Upload images sequentially and preserve the deterministic scan order.
5. Track the upload status for every image.
6. Handle rejected images, network failures, timeouts, and partial upload results.
7. Add retry handling for transient upload failures.
8. Display upload progress and a final success/failure summary in the terminal.
9. Include upload results and API responses in the JSON report.
10. Enable the upload flow only when `--upload` is specified.
