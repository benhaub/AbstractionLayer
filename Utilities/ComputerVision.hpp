/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   ComputerVision.hpp
* @details Computer vision types and functions
* @ingroup Utilities
*******************************************************************************/

#ifndef __COMPUTER_VISION_HPP__
#define __COMPUTER_VISION_HPP__

//C++
#include <cstdint>
#include <cmath>
#include <type_traits>
#include <deque>

/**
 * @enum PixelFormat
 * @brief The pixel format
 */
enum class PixelFormat : uint8_t {
    Unknown = 0, ///< Unknown
    Argb4,       ///< Alpha, Red, Green, Blue
    Argb1555,    ///< Alpha, Red, Green, Blue
    Argb2,       ///< Alpha, Red, Green, Blue
    Rgb565,      ///< Red (5 bits), Green (6 bits), Blue (5 bits)
    Rgb8,        ///< Red (8 bits), Green (8 bits), Blue (8 bits)
    Rgb4,        ///< Red (4 bits), Green (4 bits), Blue (4 bits)
    Greyscale    ///< Greyscale
};

/**
 * @enum Interpolation
 * @brief Types of resampling for image resizing
 */
enum class ImageResampling : uint8_t {
    NearestNeighbour, ///< Nearest neighbour resampling
    Bilinear,         ///< Bilinear resampling
    Polynomial,       ///< Polynomial resampling
    Box,              ///< Box resampling
};

/**
 * @struct SeedCriteria
 * @brief The criteria for reporting a seed in a buffer of pixels
 */
struct SeedCriteria {
    uint32_t minIntensity = 0;    ///< The minimum intensity of a seed
    uint32_t maxIntensity = 0;    ///< The maximum intensity of a seed
};

/**
 * @concept CompatibleBuffer
 * @brief The types that are compatible with the buffers
 */
template <typename Buffer>
concept CompatibleBuffer = std::is_same_v<Buffer, StaticString::Container &> ||
                           std::is_same_v<Buffer, std::string *> ||
                           std::is_same_v<Buffer, const StaticString::Container &> ||
                           std::is_same_v<Buffer, const std::string *>;

/**
 * @brief Convert a pixel to greyscale
 * @param[in] inputPixelFormat The pixel format of the input
 * @param[in] inputPixels The pixels to convert
 * @returns The greyscale value
 * @todo Very bad, specific use case implementation. Assumes 2 pixel input, 2 bytes per pixel.
 */
constexpr inline uint16_t ToGreyscale(const PixelFormat inputPixelFormat, const uint32_t inputPixels) {
    uint8_t grey1 = 0;
    uint8_t grey2 = 0;
    
    if (inputPixelFormat == PixelFormat::Argb4) {
        const uint16_t pixel1 = (inputPixels >> 16) & 0xFFFF;
        const uint16_t pixel2 = inputPixels & 0xFFFF;

        //Extract and normalise the RGB values
        const float pixel1Red = ((pixel1 >> 8) & 0xF) / 15.0f;
        const float pixel1Green = ((pixel1 >> 4) & 0xF) / 15.0f;
        const float pixel1Blue = (pixel1 & 0xF) / 15.0f;
        const float pixel2Red = ((pixel2 >> 8) & 0xF) / 15.0f;
        const float pixel2Green = ((pixel2 >> 4) & 0xF) / 15.0f;
        const float pixel2Blue = (pixel2 & 0xF) / 15.0f;

        //Weighted average of the RGB which gives preference to the colours seen brightest by the human eye.
        const float linearIntensity1 = 0.2126 * pixel1Red + 0.7152 * pixel1Green + 0.0722 * pixel1Blue;
        const float linearIntensity2 = 0.2126 * pixel2Red + 0.7152 * pixel2Green + 0.0722 * pixel2Blue;

        //https://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale
        if (linearIntensity1 <= 0.0031308) {
            float gammaCompressed1 = 12.92f * linearIntensity1;
            grey1 = gammaCompressed1 * 255;
        }
        else {
            float gammaCompressed1 = 1.055f * std::pow(linearIntensity1, 1.0f / 2.4f) - 0.055f;
            grey1 = gammaCompressed1 * 255;
        }

        if (linearIntensity2 <= 0.0031308) {
            float gammaCompressed2 = 12.92f * linearIntensity2;
            grey2 = gammaCompressed2 * 255;
        }
        else {
            float gammaCompressed2 = 1.055f * std::pow(linearIntensity2, 1.0f / 2.4f) - 0.055f;
            grey2 = gammaCompressed2 * 255;
        }
    }

    return (grey1 << 8) | grey2;
}

/**
 * @brief downsize an image
 * @tparam Buffer The buffer type to downsize
 * @param[in] area The area to downsize
 * @param[in] newArea The new area
 * @param[in] interpolation The interpolation method to use
 * @param[inout] buffer The image buffer to downsize
 * @post The downsize image is computed in place.
 * @returns ErrorType::Success if the image was resized
 * @returns ErrorType::NotSupported if the pixel format is not supported or the interpolation method is not supported
 */
template <typename Buffer>
requires CompatibleBuffer<Buffer>
inline ErrorType DownsizeImageImplementation(const Area &area, const Area &newArea, const ImageResampling interpolation, const PixelFormat pixelFormat, Buffer &&buffer) {
    if (PixelFormat::Greyscale == pixelFormat) {
        //https://courses.cs.vt.edu/~masc1044/L17-Rotation/ScalingNN.html
        if (ImageResampling::NearestNeighbour == interpolation) {

            for (uint32_t y = 0; y < newArea.height; y++) {

                for (uint32_t x = 0; x < newArea.width; x++) {
                    //Fill the new area with values in proportion as a percentage of the old area (e.g. We want to fill in pixel x for the new image which is at 10% of the new image width, so take the pixel that lies at 10% of the old image width).
                    const float ratioX = static_cast<float>(x) / newArea.width;
                    const float ratioY = static_cast<float>(y) / newArea.height;

                    const uint32_t srcX = std::min(static_cast<uint32_t>(std::round(ratioX * area.width)), static_cast<uint32_t>(area.width - 1));
                    const uint32_t srcY = std::min(static_cast<uint32_t>(std::round(ratioY * area.height)), static_cast<uint32_t>(area.height - 1));

                    const uint8_t pixelValue = buffer->at(area.xyToFlatIndex({srcX, srcY}));
                    buffer->at(newArea.xyToFlatIndex({x, y})) = pixelValue;
                }
            }

            buffer->resize(newArea.size());
            return ErrorType::Success;
        }
        //https://stackoverflow.com/questions/26142288/resize-an-image-with-bilinear-interpolation-without-imresize
        //https://archive.org/details/Lectures_on_Image_Processing
        else if (ImageResampling::Bilinear == interpolation) {
            const float ratioWidth = static_cast<float>(area.width) / newArea.width;
            const float ratioHeight = static_cast<float>(area.height) / newArea.height;

            for (uint32_t newImagePixelY = 0; newImagePixelY < newArea.height; newImagePixelY++) {

                for (uint32_t newImagePixelX = 0; newImagePixelX < newArea.width; newImagePixelX++) {
                    const uint32_t sourceImagePixelXWhole = newImagePixelX * ratioWidth;
                    const uint32_t sourceImagePixelYWhole = newImagePixelY * ratioHeight;
                    const float sourceImagePixelXFractional = newImagePixelX * ratioWidth - sourceImagePixelXWhole;
                    const float sourceImagePixelYFractional = newImagePixelY * ratioHeight - sourceImagePixelYWhole;

                    //The new pixel is a combination of the 4 nearest pixels (x,y), (x+1,y), (x,y+1), (x+1,y+1).
                    const float sourceImagePixel1 = buffer->at(area.xyToFlatIndex({sourceImagePixelXWhole, sourceImagePixelYWhole}));
                    const float sourceImagePixel2 = buffer->at(area.xyToFlatIndex({(sourceImagePixelXWhole + 1u), sourceImagePixelYWhole}));
                    const float sourceImagePixel3 = buffer->at(area.xyToFlatIndex({sourceImagePixelXWhole, (sourceImagePixelYWhole + 1u)}));
                    const float sourceImagePixel4 = buffer->at(area.xyToFlatIndex({(sourceImagePixelXWhole + 1u), (sourceImagePixelYWhole + 1u)}));

                    const float newImagePixel1 = sourceImagePixel1 * (1u - sourceImagePixelXFractional) * (1u - sourceImagePixelYFractional);
                    const float newImagePixel2 = sourceImagePixel2 * sourceImagePixelXFractional * (1u - sourceImagePixelYFractional);
                    const float newImagePixel3 = sourceImagePixel3 * (1u - sourceImagePixelXFractional) * sourceImagePixelYFractional;
                    const float newImagePixel4 = sourceImagePixel4 * sourceImagePixelXFractional * sourceImagePixelYFractional;
                    const uint32_t newImagePixelIndex = newArea.xyToFlatIndex({newImagePixelX, newImagePixelY});
                    buffer->at(newImagePixelIndex) = newImagePixel1 + newImagePixel2 + newImagePixel3 + newImagePixel4;
                }
            }

            buffer->resize(newArea.size());
            return ErrorType::Success;
        }
        else if (ImageResampling::Box == interpolation) {
            const float boxWidth = static_cast<float>(area.width) / newArea.width;
            const float boxHeight = static_cast<float>(area.height) / newArea.height;

            if (0 != boxWidth || 0 != boxHeight) {
                std::string resultBuffer(newArea.size(), 0);

                for (uint32_t newY = 0; newY < newArea.height; newY++) {

                    for (uint32_t newX = 0; newX < newArea.width; newX++) {
                        float accumulatedColor = 0.0f;
                        float totalWeight = 0.0f;

                        const float nextBoxY = static_cast<float>(newY) * boxHeight;
                        const float nextBoxX = static_cast<float>(newX) * boxWidth;
                        const uint32_t boxEndY = std::min(static_cast<uint32_t>(ceil(nextBoxY + boxHeight)), area.height);
                        const uint32_t boxEndX = std::min(static_cast<uint32_t>(ceil(nextBoxX + boxWidth)), area.width);

                        for (uint32_t boxY = static_cast<uint32_t>(floor(nextBoxY)); boxY < boxEndY; boxY++) {
                            const float yOverlap = std::min(static_cast<float>(boxY + 1), nextBoxY + boxHeight) - std::max(static_cast<float>(boxY), nextBoxY);

                            for (uint32_t boxX = static_cast<uint32_t>(floor(nextBoxX)); boxX < boxEndX; boxX++) {
                                const float xOverlap = std::min(static_cast<float>(boxX + 1), nextBoxX + boxWidth) - std::max(static_cast<float>(boxX), nextBoxX);
                                const float weight = xOverlap * yOverlap;

                                if (buffer->at(area.xyToFlatIndex({boxX, boxY})) != 0) {
                                    accumulatedColor += static_cast<float>(static_cast<uint8_t>(buffer->at(area.xyToFlatIndex({boxX, boxY})))) * weight;
                                }

                                totalWeight += weight;
                            }
                        }

                        if (totalWeight > 0.0f) {
                            uint8_t finalValue = static_cast<uint8_t>(std::clamp(round(accumulatedColor / totalWeight), 0.0, 255.0));
                            resultBuffer.at(newArea.xyToFlatIndex({newX, newY})) = finalValue;
                        }
                        else {
                            resultBuffer.at(newArea.xyToFlatIndex({newX, newY})) = 0;
                        }
                    }
                }

                if constexpr (std::is_same_v<Buffer, std::string *>) {
                    *buffer = std::move(resultBuffer);
                }
                else {
                    buffer->assign(resultBuffer);
                }

                return ErrorType::Success;
            }
        }
    }

    return ErrorType::NotSupported;
}
/// @copydoc downsizeImage(const Area &area, const Area &newArea, const ImageResampling interpolation, Buffer &&buffer)
inline ErrorType DownsizeImage(const Area &area, const Area &newArea, const ImageResampling interpolation, const PixelFormat pixelFormat, StaticString::Container &buffer) {
    return DownsizeImageImplementation(area, newArea, interpolation, pixelFormat, buffer);
}
/// @copydoc downsizeImage(const Area &area, const Area &newArea, const ImageResampling interpolation, Buffer &&buffer)
inline ErrorType DownsizeImage(const Area &area, const Area &newArea, const ImageResampling interpolation, const PixelFormat pixelFormat, std::string &buffer) {
    return DownsizeImageImplementation(area, newArea, interpolation, pixelFormat, &buffer);
}

/**
 * @brief Get the seed that best matches the criteria
 * @param[in] kernel The kernel to use for the seed. Origin is ignored. The pixels within the area of the kernel are potential
 *                   seeds.
 * @param[in] area The area of the buffer
 * @param[in] criteria The criteria for the seed
 * @param[in] pixelFormat The pixel format of the buffer
 * @param[inout] buffer The buffer to get the seed from
 * @param[out] seedLocation The location of the seed found
 */
inline ErrorType GetSeed(const Area &kernel, const Area &area, const SeedCriteria criteria, const PixelFormat pixelFormat, std::string_view buffer, Coordinate &seedLocation) {
    ErrorType error = ErrorType::InvalidParameter;
    //Number of connected pixels plus the total intensity.
    uint32_t currentMaxScore = 0;
    seedLocation = {0,0};

    if (kernel.size() > 0 && area.size() > 0 && kernel.size() <= area.size() && buffer.size() == area.size()) {

        if (PixelFormat::Greyscale == pixelFormat) {

            for (uint32_t y = kernel.height/2; y < area.height - kernel.height/2; y = y + kernel.height) {

                for (uint32_t x = kernel.width/2; x < area.width - kernel.width/2; x = x + kernel.width) {
                    uint32_t totalIntensity = 0;
                    const uint32_t currentKernelBoundsX = x-1 + kernel.width;
                    const uint32_t currentKernelBoundsY = y-1 + kernel.height;

                    for (uint32_t kernelY = y-1; kernelY < currentKernelBoundsY; kernelY++) {

                        for (uint32_t kernelX = x-1; kernelX < currentKernelBoundsX; kernelX++) {
                            const uint8_t pixelValue = buffer.at(area.xyToFlatIndex({kernelX, kernelY}));

                            if ((pixelValue >= criteria.minIntensity) && (pixelValue <= criteria.maxIntensity)) {
                                totalIntensity += pixelValue;
                            }
                        }
                    }

                    if (totalIntensity > currentMaxScore) {
                        currentMaxScore = totalIntensity;
                        seedLocation = {x, y};
                    }
                }
            }
        }

        error = ErrorType::Success;
    }

    return error;
}

/**
 * @brief Converts all connected pixels in the buffer starting from the coordinate given to the foreground colour
 *        if they are not the background colour.
 * @tparam Buffer The buffer type to sharpen
 * @param[in] start The coordinate to start the sharpening from
 * @param[in] pixelFormat The pixel format of the buffer
 * @param[in] background The background colour to use
 * @param[in] foreground The foreground colour to use
 * @param[inout] buffer The buffer to sharpen
 */
template <typename Buffer>
requires CompatibleBuffer<Buffer>
inline ErrorType SharpenConnectedPixelsImplementation(const Coordinate &start, const PixelFormat pixelFormat, const HexCodeColour minimumIntensity , const HexCodeColour maximumIntensity, const HexCodeColour sharpenTo, Area &area, Buffer &&buffer) {
    ErrorType error = ErrorType::InvalidParameter;

    if (buffer->size() > 0) {

        if (PixelFormat::Greyscale == pixelFormat) {
            std::vector<Coordinate> stack;
            stack.push_back(start);

            while (!stack.empty()) {
                Coordinate current = stack.back();
                stack.pop_back();

                const size_t index = area.xyToFlatIndex(current);

                if (static_cast<uint8_t>(buffer->at(index)) >= minimumIntensity && static_cast<uint8_t>(buffer->at(index)) <= maximumIntensity && static_cast<uint8_t>(buffer->at(index)) != static_cast<uint8_t>(sharpenTo)) {
                    buffer->at(index) = static_cast<uint8_t>(sharpenTo);

                    std::array<uint32_t, 8> neighbours = area.getNeighbours(current);
                    for (uint32_t neighborIndex : neighbours) {
                        const uint8_t neighborValue = buffer->at(neighborIndex);
                        if (neighborValue >= minimumIntensity && neighborValue <= maximumIntensity && neighborValue != static_cast<uint8_t>(sharpenTo)) {
                            stack.push_back(area.flatIndexToXy(neighborIndex));
                        }
                    }
                }
            }

            return ErrorType::Success;
        }
    }

    return error;
}
/// @copydoc SharpenConnectedPixelsImplementation(const Coordinate &start, const PixelFormat pixelFormat, const HexCodeColour minimumIntensity , const HexCodeColour maximumIntensity, const HexCodeColour sharpenTo, Area &area, Buffer &&buffer)
inline ErrorType SharpenConnectedPixels(const Coordinate &start, const PixelFormat pixelFormat, const HexCodeColour minimumIntensity , const HexCodeColour maximumIntensity, const HexCodeColour sharpenTo, Area &area, StaticString::Container &buffer) {
    return SharpenConnectedPixelsImplementation(start, pixelFormat, minimumIntensity , maximumIntensity, sharpenTo, area, buffer);
}
/// @copydoc SharpenConnectedPixelsImplementation(const Coordinate &start, const PixelFormat pixelFormat, const HexCodeColour minimumIntensity , const HexCodeColour maximumIntensity, const HexCodeColour sharpenTo, Area &area, Buffer &&buffer)
inline ErrorType SharpenConnectedPixels(const Coordinate &start, const PixelFormat pixelFormat, const HexCodeColour minimumIntensity , const HexCodeColour maximumIntensity, const HexCodeColour sharpenTo, Area &area, std::string &buffer) {
    return SharpenConnectedPixelsImplementation(start, pixelFormat, minimumIntensity , maximumIntensity, sharpenTo, area, &buffer);
}

template <typename Buffer>
requires CompatibleBuffer<Buffer>
inline ErrorType BinarizeImplementation(Buffer &&buffer, const PixelFormat pixelFormat) {
    ErrorType error = ErrorType::NotSupported;

    if (PixelFormat::Greyscale == pixelFormat) {

        auto otsusThreshold = [](const Buffer &buffer) -> uint8_t {
            std::array<uint32_t, 256> histogram = {0};
            float sumTotal = 0;

            for (size_t i = 0; i < buffer->size(); i++) {
                uint8_t value = buffer->at(i);
                histogram[buffer->at(i)]++;
                sumTotal += value;
            }

            float sumBackground = 0, weightBackground = 0, maxVariance = 0;
            uint8_t threshold = 0;

            for (size_t i = 0; i < histogram.size(); i++) {
                weightBackground += histogram[i];

                if (weightBackground == 0) {
                    continue;
                }

                float weightForeground = buffer->size() - weightBackground;

                if (weightForeground == 0) {
                    break;
                }

                sumBackground += (float)(i * histogram[i]);
                float meanBackground = sumBackground / weightBackground;
                float meanForeground = (sumTotal - sumBackground) / weightForeground;
                float varianceBetween = weightBackground * weightForeground * (meanBackground - meanForeground) * (meanBackground - meanForeground);
                if (varianceBetween > maxVariance) {
                    maxVariance = varianceBetween;
                    threshold = i;
                }
            }

            return threshold;
        };

        const uint8_t threshold = otsusThreshold(buffer);

        for (size_t i = 0; i < buffer->size(); i++) {
            buffer->at(i) = (buffer->at(i) > threshold) ? 255 : 0;
        }

        error = ErrorType::Success;
    }

    return error;
}
inline ErrorType Binarize(StaticString::Container &buffer, const PixelFormat pixelFormat) {
    return BinarizeImplementation(buffer, pixelFormat);
}
inline ErrorType Binarize(std::string &buffer, const PixelFormat pixelFormat) {
    return BinarizeImplementation(&buffer, pixelFormat);
}

/**
 * @brief Filter the image by veritcal strips
 * @param[in] buffer The buffer to filter
 * @param[in] area The area of the buffer
 * @param[in] pixelFormat The pixel format of the buffer
 * @param[in] stripArea The area of the strip
 * @param[in] minimumIntensity The minimum intensity of the pixels within the strip to consider for filtering
 * @param[in] maxFilterIntensity The maximum intensity that the sum of the minimum intensity pixels needs to be lower than for the strip to convert to convertTo.
 * @param[in] convertTo The colour to convert to if the pixels are below the minimum intensity
 */
template <typename Buffer>
requires CompatibleBuffer<Buffer>
inline ErrorType VerticalStripFilterImplementation(Buffer &&buffer, const Area &area, const PixelFormat pixelFormat, const Area &stripArea, const HexCodeColour minimumIntensity, const HexCodeColour maxFilterIntensity, const HexCodeColour convertTo) {
    ErrorType error = ErrorType::NotSupported;

    if (PixelFormat::Greyscale == pixelFormat) {

        if (stripArea.size() > 0 && buffer->size() == area.size()) {

            for (uint32_t x = 0; x < area.width; x = x + stripArea.width) {
                uint32_t totalIntensity = 0;
                const uint32_t currentStripBoundsX = std::min(x + stripArea.width, area.width);
                const uint32_t stripBoundsY = std::min(stripArea.height, area.height);

                for (uint32_t stripX = x; stripX < currentStripBoundsX && stripX < area.width; stripX++) {

                    for (uint32_t stripY = 0; stripY < stripBoundsY; stripY++) {
                        const uint8_t pixelValue = buffer->at(area.xyToFlatIndex({stripX, stripY}));

                        if (pixelValue >= minimumIntensity) {
                            totalIntensity += pixelValue;
                        }
                    }
                }

                if (totalIntensity <= maxFilterIntensity) {

                    for (uint32_t stripX = x; stripX < currentStripBoundsX && stripX < area.width; stripX++) {

                        for (uint32_t stripY = 0; stripY < stripBoundsY; stripY++) {
                            buffer->at(area.xyToFlatIndex({stripX, stripY})) = convertTo;
                        }
                    }
                }
            }
        }
        else {
            error = ErrorType::InvalidParameter;
        }

        error = ErrorType::Success;
    }

    return error;
}
inline ErrorType VerticalStripFilter(StaticString::Container &buffer, const Area &area, const PixelFormat pixelFormat, const Area &stripArea, const HexCodeColour minimumIntensity, const HexCodeColour maxFilterIntensity, const HexCodeColour convertTo) {
    return VerticalStripFilterImplementation(buffer, area, pixelFormat, stripArea, minimumIntensity, maxFilterIntensity, convertTo);
}
inline ErrorType VerticalStripFilter(std::string &buffer, const Area &area, const PixelFormat pixelFormat, const Area &stripArea, const HexCodeColour minimumIntensity, const HexCodeColour maxFilterIntensity, const HexCodeColour convertTo) {
    return VerticalStripFilterImplementation(&buffer, area, pixelFormat, stripArea, minimumIntensity, maxFilterIntensity, convertTo);
}

template <typename ConstBuffer, typename MutableBuffer>
requires CompatibleBuffer<ConstBuffer> && CompatibleBuffer<MutableBuffer>
inline ErrorType DilateImplementation(ConstBuffer &&undilated, const Area &area, const Area &kernel, const PixelFormat pixelFormat, const HexCodeColour toDilateMinimum, const HexCodeColour toDilateMaximum, MutableBuffer &&dilated) {
    ErrorType error = ErrorType::NotSupported;

    if (PixelFormat::Greyscale == pixelFormat) {
        if (area.size() > 0) {

            for (uint32_t y = 0; y < area.height; y++) {

                for (uint32_t x = 0; x < area.width; x++) {
                    
                    const uint32_t index = area.xyToFlatIndex({x, y});
                    const uint8_t pixelValue = static_cast<uint8_t>(undilated->at(index));

                    if (pixelValue >= toDilateMinimum && pixelValue <= toDilateMaximum) {
                        
                        for (uint32_t kernelY = 0; kernelY < kernel.height; kernelY++) {

                            for (uint32_t kernelX = 0; kernelX < kernel.width; kernelX++) {
                                
                                const uint32_t targetX = x + kernelX;
                                const uint32_t targetY = y + kernelY;
                                const uint32_t targetIndex = area.xyToFlatIndex({targetX, targetY});
                                dilated->at(targetIndex) = toDilateMaximum;
                            }
                        }
                    }
                }
            }

            error = ErrorType::Success;
        }
        else {
            error = ErrorType::InvalidParameter;
        }
    }

    return error;
}
inline ErrorType Dilate(const StaticString::Container &undilated, const Area &area, const Area &kernel, const PixelFormat pixelFormat, const HexCodeColour toDilateMinimum, const HexCodeColour toDilateMaximum, StaticString::Container &dilated) {
    return DilateImplementation(undilated, area, kernel, pixelFormat, toDilateMinimum, toDilateMaximum, dilated);
}
inline ErrorType Dilate(const std::string &undilated, const Area &area, const Area &kernel, const PixelFormat pixelFormat, const HexCodeColour toDilateMinimum, const HexCodeColour toDilateMaximum, std::string &dilated) {
    return DilateImplementation(&undilated, area, kernel, pixelFormat, toDilateMinimum, toDilateMaximum, &dilated);
}

template <typename ConstBuffer, typename MutableBuffer>
requires CompatibleBuffer<ConstBuffer> && CompatibleBuffer<MutableBuffer>
inline ErrorType FillPixelGapsImplementation(ConstBuffer &&unfilled, const Area &area, const PixelFormat pixelFormat, const uint32_t maxGapSize, const HexCodeColour gapColour, const HexCodeColour fillColour, MutableBuffer &&filled) {
    ErrorType error = ErrorType::NotSupported;

    if (PixelFormat::Greyscale == pixelFormat) {
        if (area.size() > 0 && area.size() == unfilled->size()) {

            for (uint32_t y = maxGapSize; y < area.height-1; y++) {

                for (uint32_t x = maxGapSize; x < area.width-1; x++) {
                    const uint32_t currentIndex = area.xyToFlatIndex({x, y});

                    if (unfilled->at(currentIndex) == gapColour) {

                        for (uint32_t currentGapSize = 1; currentGapSize <= maxGapSize; currentGapSize++) {
                            bool verticalGap, horizontalGap = false;
                            horizontalGap = unfilled->at(area.xyToFlatIndex({x - currentGapSize, y})) == fillColour && unfilled->at(area.xyToFlatIndex({x + currentGapSize, y})) == fillColour;

                            if (!horizontalGap) {
                                verticalGap = unfilled->at(area.xyToFlatIndex({x, y - currentGapSize})) == fillColour && unfilled->at(area.xyToFlatIndex({x, y + currentGapSize})) == fillColour;
                            }

                            if (horizontalGap || verticalGap) {
                                filled->at(currentIndex) = fillColour;
                                break;
                            }
                        }
                    }
                }
            }

            error = ErrorType::Success;
        }
        else {
            error = ErrorType::InvalidParameter;
        }
    }

    return error;
}
inline ErrorType FillPixelGaps(const StaticString::Container &unfilled, const Area &area, const PixelFormat pixelFormat, const uint32_t maxGapSize, const HexCodeColour gapColour, const HexCodeColour fillColour, StaticString::Container &filled) {
    filled->assign(std::string_view(unfilled->c_str(), unfilled->size()));
    return FillPixelGapsImplementation(unfilled, area, pixelFormat, maxGapSize, gapColour, fillColour, filled);
}
inline ErrorType FillPixelGaps(const std::string &unfilled, const Area &area, const PixelFormat pixelFormat, const uint32_t maxGapSize, const HexCodeColour gapColour, const HexCodeColour fillColour, std::string &filled) {
    filled.assign(unfilled);
    return FillPixelGapsImplementation(&unfilled, area, pixelFormat, maxGapSize, gapColour, fillColour, &filled);
}

template <typename Buffer>
requires CompatibleBuffer<Buffer>
inline ErrorType ExtractLargestIslandImplementation(Buffer &&buffer, const Area &area, const HexCodeColour islandColour) {
    ErrorType error = ErrorType::InvalidParameter;

    if (area.size() > 0 || area.size() == buffer->size()) {
        std::vector<bool> visited(area.size(), false);
        std::vector<uint32_t> largestIsland;
        
        for (uint32_t i = 0; i < area.size(); ++i) {

            if (buffer->at(i) == islandColour && !visited[i]) {
                std::vector<uint32_t> currentIsland;
                std::vector<uint32_t> queue;
                queue.push_back(i);
                visited[i] = true;
                uint32_t head = 0;

                // BFS traversal
                while (head < queue.size()) {
                    uint32_t currentIdx = queue[head++];
                    currentIsland.push_back(currentIdx);

                    Coordinate coord = { currentIdx % area.width, currentIdx / area.width };
                    // getNeighbours uses 8-connectivity (Moore Neighborhood)
                    auto neighbors = area.getNeighbours(coord);

                    for (uint32_t neighborIdx : neighbors) {

                        if (buffer->at(neighborIdx) == islandColour && !visited[neighborIdx]) {
                            visited[neighborIdx] = true;
                            queue.push_back(neighborIdx);
                        }
                    }
                }

                if (currentIsland.size() > largestIsland.size()) {
                    largestIsland = std::move(currentIsland);
                }
            }
        }

        // Remove all islands except the largest.
        for (uint32_t i = 0; i < area.size(); ++i) {
            buffer->at(i) = 0;
        }

        // Then, restore only the pixels belonging to the largest island
        for (uint32_t pixelIdx : largestIsland) {
            buffer->at(pixelIdx) = islandColour;
        }

        error = ErrorType::Success;
    }

    return error;
}
inline ErrorType ExtractLargestIsland(StaticString::Container &buffer, const Area &area, const HexCodeColour islandColour) {
    return ExtractLargestIslandImplementation(buffer, area, islandColour);
}
inline ErrorType ExtractLargestIsland(std::string &buffer, const Area &area, const HexCodeColour islandColour) {
    return ExtractLargestIslandImplementation(&buffer, area, islandColour);
}

template <typename Buffer>
requires CompatibleBuffer<Buffer>
ErrorType IslandFilterImplementation(Buffer &&buffer, const Area &area, const HexCodeColour islandColour, const HexCodeColour filterTo, const Area &minArea) {
    ErrorType error = ErrorType::PrerequisitesNotMet;
    std::vector<bool> visited(area.size(), false);

    if (area.size() > 0 && area.size() == buffer->size()) {
        error = ErrorType::Success;

        for (uint32_t i = 0; i < area.size(); ++i) {

            if (buffer->at(i) == islandColour && !visited[i]) {
                std::vector<uint32_t> currentIsland;
                std::deque<uint32_t> queue;

                visited[i] = true;
                queue.push_back(i);

                while (!queue.empty()) {
                    uint32_t currentIdx = queue.front();
                    queue.pop_front();
                    currentIsland.push_back(currentIdx);

                    Coordinate coord = area.flatIndexToXy(currentIdx);
                    auto neighbors = area.getNeighbours(coord);

                    for (uint32_t neighborIdx : neighbors) {

                        if (buffer->at(neighborIdx) == islandColour && !visited[neighborIdx]) {
                            visited[neighborIdx] = true;
                            queue.push_back(neighborIdx);
                        }
                    }
                }

                // If the island is too small, delete it immediately
                if (currentIsland.size() < minArea.size()) {

                    for (uint32_t pixelIdx : currentIsland) {
                        buffer->at(pixelIdx) = filterTo;
                    }
                }
            }
        }
    }

    return error;
}
inline ErrorType IslandFilter(StaticString::Container &buffer, const Area &area, const HexCodeColour islandColour, const HexCodeColour filterTo, const Area &minArea) {
    return IslandFilterImplementation(buffer, area, islandColour, filterTo, minArea);
}
inline ErrorType IslandFilter(std::string &buffer, const Area &area, const HexCodeColour islandColour, const HexCodeColour filterTo, const Area &minArea) {
    return IslandFilterImplementation(&buffer, area, islandColour, filterTo, minArea);
}

#endif //__COMPUTER_VISION_HPP__