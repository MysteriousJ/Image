#include "Image.h"

int passedTests = 0;
int failedTests = 0;

uint8_t expectedPixels[] = {
	255, 0, 0, 255,
	0, 255, 0, 255,
	0, 0, 255, 255,
	255, 255, 255, 255
};

Image expectedImage = {
	.pixels = expectedPixels,
	.width = 2,
	.height = 2,
};

bool closeEnough(Image a, Image b, int variability)
{
}

void testLoadImage(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		size_t byteCount = ftell(file);
		uint8_t* bytes = (uint8_t*)malloc(byteCount);
		fseek(file, 0, SEEK_SET);
		fread(bytes, 1, byteCount, file);
		fclose(file);

		Image image = loadImage(bytes, byteCount);
		size_t pixelByteCount = image.width * image.height * 4;
		if (image.width == expectedImage.width
				&& image.height == expectedImage.height
				&& memcmp(image.pixels, expectedImage.pixels, pixelByteCount) == 0)
		{
			++passedTests;
		} else {
			++failedTests;
			printf("Failed test: load %s\n", path);
		}

		ImageMetadata meta = loadImageMetadata(bytes, byteCount);
		if (meta.width == expectedImage.width && meta.height == expectedImage.height) {
			++passedTests;
		} else {
			++failedTests;
			printf("Failed test: load %s metadata\n", path);
		}
	}
}

int main()
{
	testLoadImage("test.png");

	printf("Passed %d/%d tests\n", passedTests, passedTests + failedTests);
	return (failedTests == 0)? 0 : 1;
}
