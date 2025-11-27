# Backends
**Windows:** Windows Imaging Component (native operating system API)

**MacOS/iOS:** Core Graphics (native operating system API)

**Linux:** libpng and libjpeg-turbo

# Supported Image Formats
||Windows|MacOS/iOS|Linux|
|---|-------|---------|-----|
|bmp|✓|✓| |
|gif|✓|✓| |
|ico|✓|✓| |
|jpg|✓|✓|✓|
|png|✓|✓|✓|
|tiff|✓|✓| |

Animation is out of scope.

# Usage
`#include "Image.h"` in one of your project's translation units. See the `test_*` files for libraries to link on each platform.

# API
```C++
/* Returns an Image with sRGBA 8-bits-per-color pixels (4 bytes per pixel).
 * When bytes is an invalid or unrecognized image, or when byteCount is zero,
 * pixels, width, and height are set to zero. */
Image decodeImage(const uint8_t* bytes, size_t byteCount);

/* Decodes only metadata without decoding pixels.
 * When bytes is an invalid or unrecognized image, or when byteCount is zero,
 * width and height are set to zero. */
ImageMetadata decodeImageMetadata(const uint8_t* bytes, size_t byteCount);

/* Free memory for pixels.
 * Fine to call on a zeroed image or an image that failed to decode. */
void destroyImage(Image* image);
```
