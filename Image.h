#pragma once
#include <stdint.h>
#include <stdlib.h>

struct Image
{
	uint8_t* pixels;
	uint32_t width;
	uint32_t height;
};

struct ImageMetadata
{
	uint32_t width;
	uint32_t height;
};

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

/* Mostly for internal use, but implementations are platform independent. */
uint8_t* allocateImageMemory(size_t byteCount);
bool isPng(const uint8_t* bytes, size_t byteCount);
bool isJpeg(const uint8_t* bytes, size_t byteCount);


////////// Implementation //////////
#include <string.h>

void destroyImage(Image* image)
{
	free(image->pixels);
}

uint8_t* allocateImageMemory(size_t byteCount)
{
	if (byteCount == 0) return NULL;
	return (uint8_t*)malloc(byteCount);
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

bool isWebp(const uint8_t* bytes, size_t byteCount)
{
	return byteCount > 12
		&& memcmp("RIFF", bytes, 4) == 0
		&& memcmp("WEBP", bytes+8, 4) == 0;
}

#ifdef _WIN32
#include <wincodec.h>
#include <shlwapi.h>

Image decodeImage(const uint8_t* bytes, size_t byteCount)
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

ImageMetadata decodeImageMetadata(const uint8_t* bytes, size_t byteCount)
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

Image decodeImage(const uint8_t* bytes, size_t byteCount)
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

ImageMetadata decodeImageMetadata(const uint8_t* bytes, size_t byteCount)
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
#include <webp/decode.h>

static Image decodePng(const uint8_t* bytes, size_t byteCount)
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

static Image decodeJpeg(const uint8_t* bytes, size_t byteCount)
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

static Image decodeWebp(const uint8_t* bytes, size_t byteCount)
{
	Image result = {0};
	int width, height;
	uint8_t* pixels = WebPDecodeRGBA(bytes, byteCount, &width, &height);
	if (pixels) {
		size_t pixelByteCount = width * height * 4;
		result.pixels = (uint8_t*)malloc(pixelByteCount);
		if (result.pixels) {
			memcpy(result.pixels, pixels, pixelByteCount);
			result.width = width;
			result.height = height;
		}
		WebPFree(pixels);
	}
	return result;
}

Image decodeImage(const uint8_t* bytes, size_t byteCount)
{
	Image result = {0};

	if (byteCount > 0) {
		if (isJpeg(bytes, byteCount)) {
			result = decodeJpeg(bytes, byteCount);
		} else if (isPng(bytes, byteCount)) {
			result = decodePng(bytes, byteCount);
		} else if (isWebp(bytes, byteCount)) {
			result = decodeWebp(bytes, byteCount);
		}
	}

	return result;
}

static ImageMetadata decodeJpegMetadata(const uint8_t* bytes, size_t byteCount)
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

static ImageMetadata decodePngMetadata(const uint8_t* bytes, size_t byteCount)
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

static ImageMetadata decodeWebpMetadata(const uint8_t* bytes, size_t byteCount)
{
	ImageMetadata result = {0};
	int width, height;
	if (WebPGetInfo(bytes, byteCount, &width, &height)) {
		result.width = width;
		result.height = height;
	}
	return result;
}

ImageMetadata decodeImageMetadata(const uint8_t* bytes, size_t byteCount)
{
	ImageMetadata result = {0};
	if (byteCount > 0) {
		if (isJpeg(bytes, byteCount)) {
			result = decodeJpegMetadata(bytes, byteCount);
		} else if (isPng(bytes, byteCount)) {
			result = decodePngMetadata(bytes, byteCount);
		} else if (isWebp(bytes, byteCount)) {
			result = decodeWebpMetadata(bytes, byteCount);
		}
	}
	return result;
}
#endif // Linux
