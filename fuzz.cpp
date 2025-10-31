#include "Image.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	Image image = loadImage(data, size);
	destroyImage(&image);
	loadImageMetadata(data, size);
	return 0;
}
