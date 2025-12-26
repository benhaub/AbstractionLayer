/***************************************************************************//**
* @author  Ben Haubrich
* @file    LcdAbstraction.hpp
* @details Abstraction for LCD Screens
* @ingroup Abstractions
*******************************************************************************/
#ifndef __LCD_ABSTRACTION_HPP__
#define __LCD_ABSTRACTION_HPP__

//AbstractionLayer
#include "IcCommunicationProtocol.hpp"
//C++
#include <cmath>

/**
 * @namespace LcdTypes
 * @details Types for LCD abstraction
 */
namespace LcdTypes {

    /**
     * @struct ScreenParameters
     * @brief Parameters for the screen
     */
    struct ScreenParameters {
        Area activeArea;                   ///< Active area in visible lines (resoultion width and height)
        uint16_t horizontalOffset = 0;     ///< Length of non-visible line
        uint16_t verticalOffset = 0;       ///< Number of non-visible lines
        uint16_t totalHorizontalLines = 0; ///< Length of line including visible and non-visible
        uint16_t totalVerticalLines = 0;   ///< Number of lines including visible and non-visible
        uint16_t horizontalFrontPorch = 0; ///< Horizontal front porch
        uint16_t verticalFrontPorch = 0;   ///< Vertical front porch
        uint16_t verticalPulseWidth = 0;   ///< Vertical pulse width
        uint16_t horizontalPulseWidth = 0; ///< Horizontal pulse width
        uint8_t swizzle = 0;               ///< Color channel swizzle. Adjust to get correct color output (e.g. reds appear green)
        uint8_t pixelClockDivisor = 0;     ///< Pixel clock divisor
        uint8_t pixelClockPolarity = 0;    ///< Pixel clock polarity
        uint8_t dithering = 0;             ///< Dithering setting (Used to simluate more colors on displays with less color depth i.e less than 24-bits or 16.7 million colours)
    };

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
     * @enum DesignElementType
     * @brief The type of design element
     */
    enum class DesignElementType : uint16_t {
        Unknown = 0,
        Button,
        ClearScreen,
        FreehandSketch,
        Text,
        Track
    };

    /**
     * @struct DesignElement
     * @brief A design element that can be drawn on the screen
     */
    struct DesignElement {
        DesignElementType type;    ///< The type of design element
    };

    /**
     * @struct FreehandSketch
     * @brief Draw freehand lines with touch
     */
    struct FreehandSketch : public DesignElement {
        /// @brief Constructor
        FreehandSketch() : DesignElement() {
            type = DesignElementType::FreehandSketch;
        }

        /// @brief The area of the sketch which spans from the origin specified.
        Area area;
        ///< The colour of the paper
        HexCodeColour paperColour;
        ///< The colour of the brush
        HexCodeColour brushColour;
        ///< The size of the brush
        uint16_t brushSize;
    };

    /**
     * @struct ClearScreen
     * @brief Clear the screen to a specific colour
     */
    struct ClearScreen : public DesignElement {
        /// @brief Constructor
        ClearScreen() : DesignElement() {
            type = DesignElementType::ClearScreen;
        }

        /// @brief The resulting colour of the cleared screen
        HexCodeColour colour;
    };

    /**
     * @struct Text
     * @brief Draw text on the screen
     */
    struct Text : public DesignElement {
        /// @brief Constructor
        Text() : DesignElement() {
            type = DesignElementType::Text;
        }

        /// @brief The location of the text
        Coordinate location = {0,0};
        /// @brief The colour of the text
        HexCodeColour colour = 0x00FF0000;
        /// @brief The font to use
        uint32_t font = 0;
        /// @brief The text to draw
        StaticString::Container text;
    };

    /**
     * @struct Button
     * @brief Draw a button with a label
     */
    struct Button : public DesignElement {
        /// @brief Constructor
        Button() : DesignElement() {
            type = DesignElementType::Button;
        }

        /// @brief The area of the button which spans from the origin specified.
        Area area;
        /// @brief The colour of the button
        HexCodeColour colour = 0x00FFFFFF;
        /// @brief The font to use
        uint32_t font = 0;
        /// @brief The id of the button that is used to detect presses. 0 means don't use an id
        Id id = 0;
        /// @brief The text to draw on the button
        StaticString::Container text;
    };
}

/**
 * @class LcdAbstraction
 * @brief The LCD that drives the motor.
 */
class LcdAbstraction {
    public:
    /// @brief Constructor
    LcdAbstraction() = default;
    /// @brief Destructor
    virtual ~LcdAbstraction() = default;
    /// @brief Copy Constrcutor
    LcdAbstraction(const LcdAbstraction &) = delete;
    /// @brief Move Constructor
    LcdAbstraction(LcdAbstraction &&) = default;
    /// @brief Copy Operator
    LcdAbstraction &operator = (const LcdAbstraction &) = delete;
    /// @brief Move Operator
    LcdAbstraction &operator = (LcdAbstraction &&) = default;

    /// @brief The tag for logging
    static constexpr char TAG[] = "Lcd";

    /**
     * @brief Configure the LCD.
     * @returns ErrorType::Success if the LCD was configured
     * @returns ErrorType::Failure if the LCD was not configured
     */
    virtual ErrorType configure() = 0;
    /**
     * @brief Initialize the LCD
     * @pre LcdAbstraction::configure has been called and returned ErrorType::Success
     * @returns ErrorType::Success if the LCD was initialized
     * @returns ErrorType::Failure if the LCD was not initialized
     */
    virtual ErrorType init() = 0;
    /**
     * @brief Reset the LCD
     * @returns ErrorType::Success if the LCD was reset
     * @returns ErrorType::Failure if the LCD was not reset
     */
    virtual ErrorType reset() = 0;
    /**
     * @brief Start a new design
     */
    virtual ErrorType startDesign() = 0;
    /**
     * @brief Add a design element to the screen
     * @param[in] element The design element to add
     * @returns ErrorType::Success if the design element was added
     * @returns ErrorType::Failure if the design element was not added
     */
    virtual ErrorType addDesignElement(const LcdTypes::DesignElement &element) = 0;
    /**
     * @brief flush the design elements to the screen
     * @returns ErrorType::Success if the design elements were flushed
     * @returns ErrorType::Failure if the design elements were not flushed
     */
    virtual ErrorType endDesign() = 0;
    /**
     * @brief Copy the contents of the screen within the area specified to the buffer.
     * @param[out] buffer The buffer to copy the screen contents to
     * @returns ErrorType::Success if the screen was copied
     * @returns ErrorType::Failure if the screen was not copied
     * @post The buffer is not resized if the call is unsuccessful
     * @post The buffer is resized to the number of bytes copied if the call is successful
     */
    virtual ErrorType copyScreen(StaticString::Container &buffer, const Area &area, const LcdTypes::PixelFormat pixelFormat) = 0;
    /// @copydoc LcdAbstraction::copyScreen(StaticString::Container &buffer)
    virtual ErrorType copyScreen(std::string &buffer, const Area &area, const LcdTypes::PixelFormat pixelFormat) = 0;
    /**
     * @brief Wait for touches on the design elements with the given IDs
     * @param[in] designElements The IDs of the design elements to wait for touches on
     * @param[in] timeout The timeout in milliseconds
     * @returns ErrorType::Success if the design elements were touched
     * @returns ErrorType::NoData if no design element IDs were provided
     * @returns ErrorType::Timeout if the design elements were not touched within the timeout
     */
    virtual ErrorType waitForTouches(std::initializer_list<Id> designElements, Milliseconds timeout) = 0;

    /// @brief Get the LCD parameters
    virtual constexpr LcdTypes::ScreenParameters screenParameters() = 0;
};

#endif //__HBRIDGE_ABSTRACTION_HPP__