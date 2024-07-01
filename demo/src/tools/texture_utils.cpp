// Project includes
#include "graphics/dx12_backend.h"
#include "tools/security.h"

// Lib Tiff
#if defined(SUPPORT_TIFF)
#include <libtiff/tiffio.h>
#endif

#if defined(SUPPORT_SPNG)
#include <spng.h>
#endif

// System includes
#include <iostream>
#include <fstream>
#include <vector>



#if defined(SUPPORT_SPNG)
GraphicsBuffer load_image_to_graphics_buffer_png(GraphicsDevice device, const char* texturePath, uint32_t& width, uint32_t& height, TextureFormat& format)
{
	// Load the file to memory
	std::ifstream fileStream(texturePath, std::ios::binary | std::ios::ate);
	std::streamsize size = fileStream.tellg();
	fileStream.seekg(0, std::ios::beg);
	std::vector<char> buffer(size);
	fileStream.read(buffer.data(), size);

	// Create a decoder context
	spng_ctx* ctx = spng_ctx_new(0);

	// Load the data to the decoder
	spng_set_png_buffer(ctx, buffer.data(), buffer.size());

	// Get the dimensions
	spng_ihdr ihdr;
	spng_get_ihdr(ctx, &ihdr);
	width = ihdr.width;
	height = ihdr.height;

	// Define the decopressed size
	size_t decodedBufferSize;
	spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &decodedBufferSize);

	// Buffer that will hold all the data
	std::vector<char> rawBuffer;
	rawBuffer.resize(decodedBufferSize);
	spng_decode_image(ctx, rawBuffer.data(), decodedBufferSize, SPNG_FMT_RGBA8, 0);

	// Free the decoder
	spng_ctx_free(ctx);

	// Allocate the upload texture
	GraphicsBuffer textureBuffer = d3d12::graphics_resources::create_graphics_buffer(device, width * height * 4, 4, GraphicsBufferType::Upload);
	format = TextureFormat::R8G8B8A8_UNorm;

	// Copy everything to it
	d3d12::graphics_resources::set_buffer_data(textureBuffer, rawBuffer.data(), rawBuffer.size());

	// return it
	return textureBuffer;
}
#endif

#if defined(SUPPORT_TIFF)
void get_adequate_format(uint16_t channelSize, uint16_t channelCount, uint16_t channelFormat, TextureFormat& outputFormat, uint16_t& outputChannelCount)
{
	if (channelCount == 1)
	{
		if (channelFormat == SAMPLEFORMAT_IEEEFP)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32_Float;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16_Float;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8_UNorm;
		}
		else if (channelFormat == SAMPLEFORMAT_INT)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32_SInt;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16_SInt;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8_SInt;
		}
		else if (channelFormat == SAMPLEFORMAT_UINT)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32_UInt;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16_UInt;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8_UInt;
		}
		outputChannelCount = 1;
	}
	else if (channelCount == 2)
	{
		if (channelFormat == SAMPLEFORMAT_IEEEFP)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32G32_Float;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16G16_Float;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8G8_UNorm;
		}
		else if (channelFormat == SAMPLEFORMAT_INT)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32G32_SInt;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16G16_SInt;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8G8_SInt;
		}
		else if (channelFormat == SAMPLEFORMAT_UINT)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32G32_UInt;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16G16_UInt;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8G8_UInt;
		}
		outputChannelCount = 2;
	}
	else if (channelCount == 3 || channelCount == 4)
	{
		if (channelFormat == SAMPLEFORMAT_IEEEFP)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32G32B32A32_Float;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16G16B16A16_Float;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8G8B8A8_UNorm;
		}
		else if (channelFormat == SAMPLEFORMAT_INT)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32G32B32A32_SInt;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16G16B16A16_SInt;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8G8B8A8_SInt;
		}
		else if (channelFormat == SAMPLEFORMAT_UINT)
		{
			if (channelSize == 4)
				outputFormat = TextureFormat::R32G32B32A32_UInt;
			else if (channelSize == 2)
				outputFormat = TextureFormat::R16G16B16A16_UInt;
			else if (channelSize == 1)
				outputFormat = TextureFormat::R8G8B8A8_UInt;
		}
		outputChannelCount = 4;
	}
}

void tif_read_error(const char* a , const char* b, va_list)
{
	assert_fail_msg("tif failure");
}

GraphicsBuffer load_image_to_graphics_buffer_tiff(GraphicsDevice device, const char* texturePath, uint32_t& width, uint32_t& height, TextureFormat& textureFormat)
{
	// Set the error callback
	TIFFSetErrorHandler(&tif_read_error);

	// Open the file
	TIFF* tif = TIFFOpen(texturePath, "r");
	assert_msg(tif != nullptr, "Could not find tiff file.");

	// Read the resource's dimensions
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

	// Number of channels per pixel
	uint16_t channelCount, bitsPerChannel, channelFormat;
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channelCount);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerChannel);
	uint16_t channelSize = bitsPerChannel / 8;
	TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &channelFormat);
	if (channelSize == 1 && channelCount == 3)
		channelFormat = SAMPLEFORMAT_UINT;

	// Get the output format
	uint16_t actualChannelCount;
	get_adequate_format(channelSize, channelCount, channelFormat, textureFormat, actualChannelCount);

	// Allocate the upload texture
	const uint16_t pixelSize = actualChannelCount * channelSize;
	const uint32_t imageSize = width * height * pixelSize;
	GraphicsBuffer textureBuffer = d3d12::graphics_resources::create_graphics_buffer(device, imageSize, pixelSize, GraphicsBufferType::Upload);
	char* bufferCPU = d3d12::graphics_resources::allocate_cpu_buffer(textureBuffer);

	// Read the image
	tsize_t lineSize = TIFFScanlineSize(tif);

	assert(lineSize == width * (channelSize * channelCount));

	// Allocate a buffer for line reading
	char* image = (char*)malloc(width * channelSize * channelCount);
	for (uint32_t channelIdx = 0; channelIdx < channelCount; ++channelIdx)
	{
		// For each line
		for (uint32_t y = 0; y < height; y++)
		{
			TIFFReadScanline(tif, image, y, channelIdx);
			for (uint32_t x = 0; x < width; x++)
			{
				uint32_t inputOffset = channelSize * channelCount * x;
				uint32_t outputOffset = pixelSize * (x + y * width);
				memcpy(bufferCPU + outputOffset, image + inputOffset, channelSize * channelCount);
			}
		}
	}

	// free the buffer
	free(image);

	// Release the CPU view
	d3d12::graphics_resources::release_cpu_buffer(textureBuffer);

	// Close the tiff file
	TIFFClose(tif);

	// Return the created graphics buffer
	return textureBuffer;
}
#endif
