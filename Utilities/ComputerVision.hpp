/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   ComputerVision.hpp
* @details Computer vision types and functions
* @ingroup Utilities
*******************************************************************************/

#ifndef __COMPUTER_VISION_HPP__
#define __COMPUTER_VISION_HPP__

#include <cstdint>
#include <cmath>
#include <type_traits>

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
requires std::is_same_v<Buffer, StaticString::Container &> || std::is_same_v<Buffer, std::string *>
inline ErrorType downsizeImageImplementation(const Area &area, const Area &newArea, const ImageResampling interpolation, const PixelFormat pixelFormat, Buffer &&buffer) {
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
inline ErrorType downsizeImage(const Area &area, const Area &newArea, const ImageResampling interpolation, const PixelFormat pixelFormat, StaticString::Container &buffer) {
    return downsizeImageImplementation(area, newArea, interpolation, pixelFormat, buffer);
}
/// @copydoc downsizeImage(const Area &area, const Area &newArea, const ImageResampling interpolation, Buffer &&buffer)
inline ErrorType downsizeImage(const Area &area, const Area &newArea, const ImageResampling interpolation, const PixelFormat pixelFormat, std::string &buffer) {
    return downsizeImageImplementation(area, newArea, interpolation, pixelFormat, &buffer);
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
requires std::is_same_v<Buffer, StaticString::Container &> || std::is_same_v<Buffer, std::string *>
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
requires std::is_same_v<Buffer, StaticString::Container &> || std::is_same_v<Buffer, std::string *>
inline ErrorType BinarizeImplementation(Buffer &&buffer, const PixelFormat pixelFormat) {
    ErrorType error = ErrorType::NotSupported;

    if (PixelFormat::Greyscale == pixelFormat) {

        auto otsusThreshold = [](const Buffer &buffer) -> uint8_t {
            const uint32_t total = buffer->size();
            std::array<uint8_t, 256> hist = {0};

            for (uint32_t i = 0; i < total; i++) {
                hist[buffer->at(i)]++;
            }

            float sumTotal = 0;

            for (auto itr = hist.begin(); itr != hist.end(); itr++) {
                sumTotal += std::distance(itr, hist.end()) * *itr;
            }

            float sumBackground = 0, weightBackground = 0, maxVar = 0;
            uint8_t threshold = 0;

            for (int i = 0; i < 256; i++) {
                weightBackground += hist[i];

                if (weightBackground == 0) {
                    continue;
                }

                float weightForeground = total - weightBackground;

                if (weightForeground == 0) {
                    break;
                }

                sumBackground += (float)(i * hist[i]);
                float meanBackground = sumBackground / weightBackground;
                float meanForeground = (sumTotal - sumBackground) / weightForeground;
                float varianceBetween = weightBackground * weightForeground * (meanBackground - meanForeground) * (meanBackground - meanForeground);
                if (varianceBetween > maxVar) {
                    maxVar = varianceBetween;
                    threshold = i;
                }
            }

            return threshold;
        };

        const uint8_t threshold = otsusThreshold(buffer);

        for (uint32_t i = 0; i < buffer->size(); i++) {
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

#endif //__COMPUTER_VISION_HPP__