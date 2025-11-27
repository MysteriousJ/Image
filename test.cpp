#include "Image.h"
#include <stdio.h>

int passedTests = 0;
int failedTests = 0;

uint8_t expectedPixels[] = {
	255, 0, 0, 255,
	0, 255, 0, 255,
	0, 0, 255, 255,
	255, 255, 255, 255
};

Image expectedImage = {
	expectedPixels,
	2,
	2,
};

bool closeEnough(Image a, Image b, int tolerance)
{
	if (a.width != b.width || a.height != b.height) return false;
	for (int i=0; i < sizeof(expectedPixels); ++i) {
		if (abs(a.pixels[i] - b.pixels[i]) > tolerance) {
			return false;
		}
	}
	return true;
}

void testLoadImage(const char* path, int tolerance)
{
	FILE* file = fopen(path, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		size_t byteCount = ftell(file);
		uint8_t* bytes = (uint8_t*)malloc(byteCount);
		fseek(file, 0, SEEK_SET);
		fread(bytes, 1, byteCount, file);
		fclose(file);

		Image image = decodeImage(bytes, byteCount);
		size_t pixelByteCount = image.width * image.height * 4;
		if (closeEnough(image, expectedImage, tolerance))
		{
			++passedTests;
		} else {
			++failedTests;
			printf("Failed test: load %s\n", path);
		}
		destroyImage(&image);

		ImageMetadata meta = decodeImageMetadata(bytes, byteCount);
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
	testLoadImage("./resources/test.jpg", 2);
	testLoadImage("./resources/test.png", 0);
	testLoadImage("./resources/test.webp", 0);

	printf("Passed %d/%d tests\n", passedTests, passedTests + failedTests);
	return (failedTests == 0)? 0 : 1;
}
