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

uint8_t* allocateImageMemory(size_t byteCount)
{
	if (byteCount == 0) return NULL;
	return (uint8_t*)malloc(byteCount);
}

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
#include <wincodec.h>
#include <shlwapi.h>

Image loadImage(const uint8_t* bytes, size_t byteCount)
{
	CoInitialize(NULL);
	Image result = {0};
	IStream* stream = SHCreateMemStream(bytes, (UINT)byteCount);
	IWICImagingFactory* imagingFactory = NULL;
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&imagingFactory);

	IWICBitmapDecoder* decoder = NULL;
	IWICBitmapFrameDecode* frame = NULL;
	imagingFactory->CreateDecoderFromStream(stream, NULL, WICDecodeMetadataCacheOnLoad, &decoder);
	if (decoder) decoder->GetFrame(0, &frame);

	IWICFormatConverter* converter = NULL;
	uint32_t width = 0;
	uint32_t height = 0;
	if (frame) {
		imagingFactory->CreateFormatConverter(&converter);
		converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, 0, 0, WICBitmapPaletteTypeCustom);
		converter->GetSize(&width, &height);
	}

	UINT imageByteCount = width * height * 4;
	result.pixels = allocateImageMemory(imageByteCount);
	if (result.pixels) {
		result.width = width;
		result.height = height;
		UINT stride = width * 4;
		converter->CopyPixels(NULL, stride, imageByteCount, result.pixels);
	}

	if (frame) frame->Release();
	if (decoder) decoder->Release();
	if (converter) converter->Release();
	imagingFactory->Release();
	stream->Release();
	return result;
}

ImageMetadata loadImageMetadata(const uint8_t* bytes, size_t byteCount)
{
	CoInitialize(NULL);
	ImageMetadata result = {0};
	IStream* stream = SHCreateMemStream(bytes, (UINT)byteCount);
	IWICImagingFactory* imagingFactory = NULL;
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&imagingFactory);

	IWICBitmapDecoder* decoder = NULL;
	IWICBitmapFrameDecode* frame = NULL;
	imagingFactory->CreateDecoderFromStream(stream, NULL, WICDecodeMetadataCacheOnLoad, &decoder);
	if (decoder) decoder->GetFrame(0, &frame);
	if (frame) {
		frame->GetSize(&result.width, &result.height);
		frame->Release();
	}

	if (decoder) decoder->Release();
	imagingFactory->Release();
	stream->Release();
	return result;
}
#endif // _WIN32

#ifdef __APPLE__
#include <CoreImage/CoreImage.h>

Image loadImage(const uint8_t* bytes, size_t byteCount)
{
	Image result = {0};
	CFDataRef data = CFDataCreate(0, bytes, byteCount);
	CGImageSourceRef imageSource = CGImageSourceCreateWithData(data, NULL);
	CGImage* image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);

	uint32_t width = CGImageGetWidth(image);
	uint32_t height = CGImageGetHeight(image);
	size_t imageByteCount = width * height * 4;
	result.pixels = allocateImageMemory(imageByteCount);
	if (result.pixels) {
		result.width = width;
		result.height = height;
		memset(result.pixels, 0, imageByteCount);
		CGColorSpace* colorSpace = CGColorSpaceCreateDeviceRGB();
		CGContext* cgContext = CGBitmapContextCreate(result.pixels, width, height, 8, 4 * width, colorSpace, kCGImageAlphaPremultipliedLast);
		CGRect rect = CGRectMake(0, 0, width, height);
		CGContextDrawImage(cgContext, rect, image);

		CGColorSpaceRelease(colorSpace);
		CGContextRelease(cgContext);
	}

	CGImageRelease(image);
	CFRelease(imageSource);
	CFRelease(data);
	return result;
}

ImageMetadata loadImageMetadata(const uint8_t* bytes, size_t byteCount)
{
	ImageMetadata result = {0};
	CFDataRef data = CFDataCreate(0, bytes, byteCount);
	CGImageSourceRef imageSource = CGImageSourceCreateWithData(data, NULL);
	CGImage* image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);

	result.width = CGImageGetWidth(image);
	result.height = CGImageGetHeight(image);

	CGImageRelease(image);
	CFRelease(imageSource);
	CFRelease(data);
	return result;
}
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
		result.pixels = allocateImageMemory(PNG_IMAGE_SIZE(png));
	}
	if (result.pixels) {
		result.width = png.width;
		result.height = png.height;
		png_image_finish_read(&png, NULL, result.pixels, 0, 0);
	}
	png_image_free(&png);
	return result;
}

Image loadJpeg(const uint8_t* bytes, size_t byteCount)
{
	Image result = {0};
	tjhandle turbojpeg = tj3Init(TJINIT_DECOMPRESS);
	uint32_t width  = 0;
	uint32_t height = 0;
	if (tj3DecompressHeader(turbojpeg, bytes, byteCount) == 0) {
		width  = tj3Get(turbojpeg, TJPARAM_JPEGWIDTH);
		height = tj3Get(turbojpeg, TJPARAM_JPEGHEIGHT);
		result.pixels = allocateImageMemory(width * height * 4);
	}
	if (result.pixels) {
		result.width  = width;
		result.height = height;
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
