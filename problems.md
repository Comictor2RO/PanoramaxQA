# Issues encountered

## 1. Unknown XMP namespace for pitch and roll

### Symptom

The parser stopped before reading EXIF metadata for images that did not have `pitch` and `roll` fields. The application displayed the error:

```text
No namespace info available for XMP prefix `Camera'
```

### Cause

The parser attempted to look up the following XMP tags:

```text
Xmp.Camera.Pitch
Xmp.Camera.Roll
```

The `Camera` namespace was not registered in Exiv2. Creating the XMP key threw an exception, and execution left the function before GPS data could be parsed from EXIF.

### Resolution

XMP parsing was treated as optional and isolated in a `try/catch` block. If the XMP namespace or tags are missing, EXIF parsing continues normally.

Missing `pitch` and `roll` values must not invalidate the image. These fields retain their default values when they are unavailable.

## 2. Traditional tools do not expose C2PA/CBOR metadata

### Symptom

`exiv2 -pa` displayed EXIF and XMP metadata, but it could not directly read values such as:

```text
[CBOR] ActionsSoftwareAgentName: BytePlus_ModelArk
```

A detector based only on `Exif.Image.Software` and `Xmp.xmp.CreatorTool` could not identify this marker.

### Cause

C2PA uses manifests and JUMBF/CBOR data for provenance. This data is not equivalent to EXIF or XMP tags, and the local version of Exiv2 did not provide a C++ API for reading it.

### Resolution

The project uses the `c2pa-cpp` library to open the manifest with `c2pa::Reader::from_asset()`, read the manifest as JSON, and search for C2PA indicators separately from the EXIF parser.

`detectAiMetadata()` remains a fallback for EXIF/XMP. C2PA is handled by a separate detector because it requires reading and validating the manifest, not just searching metadata tags.
