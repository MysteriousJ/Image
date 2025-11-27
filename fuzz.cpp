#include "Image.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	Image image = decodeImage(data, size);
	destroyImage(&image);
	decodeImageMetadata(data, size);
	return 0;
}
