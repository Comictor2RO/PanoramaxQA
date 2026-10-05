# JSON report example

```json
{
  "summary": {
    "total_images": 10,
    "passed_images": 8,
    "failed_images": 2,
    "sequence_passed": false
  },
  "images": [
    {
      "file_path": "images/photo001.jpg",
      "width": 4000,
      "height": 3000,
      "timestamp": "2026-10-05T12:30:00Z",
      "gps": {
        "valid": true,
        "latitude": 44.4268,
        "longitude": 26.1025,
        "altitude": 80.0,
        "accuracy": 3.2
      },
      "orientation": {
        "heading": 135.0,
        "pitch": 0.0,
        "roll": 0.0
      },
      "quality": {
        "passed": true,
        "brightness": 121.4,
        "sharpness": 450.2,
        "blur_score": 0.91,
        "gps_score": 1.0
      },
      "ai": {
        "verdict": "not_detected",
        "confidence": 0.0,
        "indicators": []
      },
      "blur-positon": {
        "x": [12.5, 12, 25],
        "y": [30, 12, 5],
        "width": 30,
        "height": 30
      }
      "issues": []
    }
  ],
  "sequence": {
    "passed": false,
    "issues": [
      {
        "type": "gps_jump",
        "first_image": "images/photo004.jpg",
        "second_image": "images/photo005.jpg",
        "value": 1250.5
      }
    ]
  }
}
```
