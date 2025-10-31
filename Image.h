#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct Image
{
	uint8_t* pixels; // RGBA
	uint32_t width;
	uint32_t height;
};

struct ImageMetadata
{
	uint32_t width;
	uint32_t height;
};

void destroyImage(Image* image)
{
	free(image->pixels);
	memset(image, 0, sizeof(Image));
}

bool isPng(const uint8_t* bytes, size_t byteCount)
{
	uint8_t pngMagicNumber[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	return byteCount > sizeof(pngMagicNumber) 
		&& memcmp(pngMagicNumber, bytes, sizeof(pngMagicNumber)) == 0;
}

bool isJpeg(const uint8_t* bytes, size_t byteCount)
{
	uint8_t jpegMagicNumber[] = {0xFF, 0xD8, 0xFF};
	return byteCount > sizeof(jpegMagicNumber)
		&& memcmp(jpegMagicNumber, bytes, sizeof(jpegMagicNumber)) == 0;
}

#ifdef _WIN32
#endif // _WIN32

#ifdef __APPLE__
#endif // __APPLE__ 

#if defined(__linux__) && !defined(__ANDROID__)
#include <turbojpeg.h>
#include <png.h>

Image loadPng(const uint8_t* bytes, size_t byteCount)
{
	Image result = {0};
	png_image png = {0};
	png.version = PNG_IMAGE_VERSION;
	if (png_image_begin_read_from_memory(&png, bytes, byteCount) != 0) {
		png.format = PNG_FORMAT_RGBA;
		result.pixels = (uint8_t*)malloc(PNG_IMAGE_SIZE(png));
	}
	if (result.pixels) {
		result.width = png.width;
		result.height = png.height;
		png_color background = {255, 255, 255};
		png_image_finish_read(&png, &background, result.pixels, 0, 0);
	}
	return result;
}

Image loadJpeg(const uint8_t* bytes, size_t byteCount)
{
	Image result = {0};
	tjhandle turbojpeg = tj3Init(TJINIT_DECOMPRESS);
	if (tj3DecompressHeader(turbojpeg, bytes, byteCount) == 0) {
		result.width  = tj3Get(turbojpeg, TJPARAM_JPEGWIDTH);
		result.height = tj3Get(turbojpeg, TJPARAM_JPEGHEIGHT);
		result.pixels = (uint8_t*)malloc(result.width * result.height * 4);
		tj3Decompress8(turbojpeg, bytes, byteCount, result.pixels, 0, TJPF_RGBA);
	}
	tjDestroy(turbojpeg);
	return result;
}

Image loadImage(const uint8_t* bytes, size_t byteCount)
{
	Image result = {0};

	if (byteCount > 0) {
		if (isJpeg(bytes, byteCount)) {
			result = loadJpeg(bytes, byteCount);
		} else if (isPng(bytes, byteCount)) {
			result = loadPng(bytes, byteCount);
		}
	}

	return result;
}

ImageMetadata loadJpegMetadata(const uint8_t* bytes, size_t byteCount)
{
	ImageMetadata result = {0};
	tjhandle turbojpeg = tj3Init(TJINIT_DECOMPRESS);
	if (tj3DecompressHeader(turbojpeg, bytes, byteCount) == 0) {
		result.width = tj3Get(turbojpeg, TJPARAM_JPEGWIDTH);
		result.height = tj3Get(turbojpeg, TJPARAM_JPEGHEIGHT);
	}
	tjDestroy(turbojpeg);
	return result;
}

ImageMetadata loadPngMetadata(const uint8_t* bytes, size_t byteCount)
{
	ImageMetadata result = {0};
	png_image png = {.version = PNG_IMAGE_VERSION};
	if (png_image_begin_read_from_memory(&png, bytes, byteCount) != 0) {
		result.width = png.width;
		result.height = png.height;
	}
	png_image_free(&png);
	return result;
}

ImageMetadata loadImageMetadata(const uint8_t* bytes, size_t byteCount)
{
	ImageMetadata result = {0};
	if (byteCount > 0) {
		if (isJpeg(bytes, byteCount)) {
			result = loadJpegMetadata(bytes, byteCount);
		} else if (isPng(bytes, byteCount)) {
			result = loadPngMetadata(bytes, byteCount);
		}
	}
	return result;
}
#endif // Linux
