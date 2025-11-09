# Usage
`#include "Image.h"` in one of your project's translation units. See the `test_*` files for libraries to link on each platform.

# Supported Image Formats
||Windows|MacOS/iOS|Linux|
|---|-------|---------|-----|
|bmp|✓|✓| |
|gif|✓|✓| |
|ico|✓|✓| |
|jpg|✓|✓|✓|
|png|✓|✓|✓|
|tiff|✓|✓| |

* Uses Windows Imaging Component on Windows (native operating system API)
* Uses Core Graphics on MacOS and iOS (native operating system API)
* Uses libpng and libjpeg-turbo on Linux
