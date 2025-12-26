//AbstractionLayer
#include <Types.hpp>

/**
 * @namespace Bridgetek81xTypes
 * @details Types for Bridgetek BT81x graphics controller
 */
namespace Bridgetek81xTypes {
    /**
     * @enum BaseAddresses
     * @brief Base addresses for writing to registers.
     * @details Commands are issues by writing data to the appropriate address. All command values are offsets relative to
     *          a base address
     * @sa Section 5, Memory Map Bt81X datasheet
     */
    enum class BaseAddresses : uint32_t {
        GeneralPurposeGraphicsRam  = 0x0,      ///< RAM_G
        RomCodesFontTableAndBitMap = 0x200000, ///< ROM
        DisplayListRam             = 0x300000, ///< RAM_DL
        Registers                  = 0x302000, ///< RAM_REG
        CommandBuffer              = 0x308000, ///< RAM_CMD
        Flash                      = 0x800000  ///< Flash memory
    };
    /**
     * @enum AddressSpace
     * @brief Address space types for BT81x
     */
    enum class AddressSpace : uint32_t {
        GeneralPurposeGraphicsRam  = 1024*1024,    ///< 1MiB RAM_G
        RomCodesFontTableAndBitMap = 1024*1024,    ///< 1MiB ROM
        DisplayListRam             = 8*1024,       ///< 8KiB RAM_DL
        Registers                  = 4*1024,       ///< 4KiB RAM_REG
        CommandBuffer              = 4*1024,       ///< 4KiB RAM_CMD
        Flash                      = 256*1024*1024 ///< 256MiB Flash memory
    };
    /**
     * @enum HostCommands
     * @brief Host commands that can be sent to BT81x
     */
    enum class HostCommands : uint8_t {
        Active =           0x00, ///< Set to active mode.
        Standby =          0x41, ///< Set to standby mode.
        Sleep =            0x42, ///< Set to sleep mode.
        PowerDown =        0x50, ///< Power down
        ClockInternal =    0x48, ///< Select PLL input from internal relaxation oscillator.
        ClockExternal =    0x44, ///< Select PLL input from the external crystal (if your PCB has connected it)
        ClockSelect =      0x62, ///< Set the system clock frequency
        ResetPulse =       0x68, ///< Reset the BT81xx core
        Unknown =          0x69  ///< Unknown host command
    };

    /**
     * @enum Commands
     * @brief Commands that can be sent to te BT81x
     */
    enum class Commands : uint32_t {
        Unknown          = 0x0,
        Append           = 0xFFFFFF1E,
        BgColor          = 0xFFFFFF09,
        Button           = 0xFFFFFF0D,
        Calibrate        = 0xFFFFFF15,
        Clock            = 0xFFFFFF14,
        Coldstart        = 0xFFFFFF32,
        Crc              = 0xFFFFFF18,
        Dial             = 0xFFFFFF2D,
        DisplayListStart = 0xFFFFFF00,
        FgColor          = 0xFFFFFF0A,
        Gauge            = 0xFFFFFF13,
        Getmatrix        = 0xFFFFFF33,
        Getprops         = 0xFFFFFF25,
        Getptr           = 0xFFFFFF23,
        Gradcolor        = 0xFFFFFF34,
        Gradient         = 0xFFFFFF0B,
        Inflate          = 0xFFFFFF22,
        Inflate2         = 0xFFFFFF50,
        Interrupt        = 0xFFFFFF02,
        Keys             = 0xFFFFFF0E,
        Loadidentity     = 0xFFFFFF26,
        Loadimage        = 0xFFFFFF24,
        Logo             = 0xFFFFFF31,
        Mediafifo        = 0xFFFFFF39,
        Memcpy           = 0xFFFFFF1D,
        Memcrc           = 0xFFFFFF18,
        Memset           = 0xFFFFFF1B,
        Memwrite         = 0xFFFFFF1A,
        Memzero          = 0xFFFFFF1C,
        Number           = 0xFFFFFF2E,
        Playvideo        = 0xFFFFFF3A,
        Progress         = 0xFFFFFF0F,
        Regread          = 0xFFFFFF19,
        Rotate           = 0xFFFFFF29,
        Scale            = 0xFFFFFF28,
        Screensaver      = 0xFFFFFF2F,
        Scrollbar        = 0xFFFFFF11,
        Setbitmap        = 0xFFFFFF43,
        Setfont          = 0xFFFFFF2B,
        Setmatrix        = 0xFFFFFF2A,
        Setrotate        = 0xFFFFFF36,
        Sketch           = 0xFFFFFF30,
        Slider           = 0xFFFFFF10,
        Snapshot         = 0xFFFFFF1F,
        Snapshot2        = 0xFFFFFF37,
        Spinner          = 0xFFFFFF16,
        Stop             = 0xFFFFFF17,
        Swap             = 0xFFFFFF01,
        Text             = 0xFFFFFF0C,
        Toggle           = 0xFFFFFF12,
        Track            = 0xFFFFFF2C,
        Translate        = 0xFFFFFF27,
        Videoframe       = 0xFFFFFF41,
        Videostart       = 0xFFFFFF40,
        Romfont          = 0xFFFFFF3F,
        Flasherase       = 0xFFFFFF44,
        Flashwrite       = 0xFFFFFF45,
        Flashread        = 0xFFFFFF46,
        Flashupdate      = 0xFFFFFF47,
        Flashdetach      = 0xFFFFFF48,
        Flashattach      = 0xFFFFFF49,
        Flashfast        = 0xFFFFFF4A,
        Flashspidesel    = 0xFFFFFF4B,
        Flashspitx       = 0xFFFFFF4C,
        Flashspirx       = 0xFFFFFF4D,
        Flashsource      = 0xFFFFFF4E,
        Clearcache       = 0xFFFFFF4F,
        VideoStartF      = 0xFFFFFF5F,
        StartAnimation   = 0xFFFFFF53
    };
    /**
     * @enum GraphicsEngineRegisters
     * @brief Registers to control the graphics engine
     */
    enum class GraphicsEngineRegisters : uint32_t {
        Unknown            = 0x00,
        Cspread            = static_cast<uint32_t>(BaseAddresses::Registers) + 0x68,
        Dither             = static_cast<uint32_t>(BaseAddresses::Registers) + 0x60,
        DisplayListSwap    = static_cast<uint32_t>(BaseAddresses::Registers) + 0x54,
        Hcycle             = static_cast<uint32_t>(BaseAddresses::Registers) + 0x2C,
        Hoffset            = static_cast<uint32_t>(BaseAddresses::Registers) + 0x30,    
        Hsize              = static_cast<uint32_t>(BaseAddresses::Registers) + 0x34,
        Hsync0             = static_cast<uint32_t>(BaseAddresses::Registers) + 0x38,
        Hsync1             = static_cast<uint32_t>(BaseAddresses::Registers) + 0x3C,
        Outbits            = static_cast<uint32_t>(BaseAddresses::Registers) + 0x5C,
        PixelClock         = static_cast<uint32_t>(BaseAddresses::Registers) + 0x70,
        PixelClockPolarity = static_cast<uint32_t>(BaseAddresses::Registers) + 0x6C,
        Play               = static_cast<uint32_t>(BaseAddresses::Registers) + 0x8C,
        PlaybackFormat     = static_cast<uint32_t>(BaseAddresses::Registers) + 0xC4,
        PlaybackFreq       = static_cast<uint32_t>(BaseAddresses::Registers) + 0xC0,
        PlaybackLength     = static_cast<uint32_t>(BaseAddresses::Registers) + 0xB8,
        PlaybackLoop       = static_cast<uint32_t>(BaseAddresses::Registers) + 0xC8,
        PlaybackPlay       = static_cast<uint32_t>(BaseAddresses::Registers) + 0xCC,
        PlaybackReadPtr    = static_cast<uint32_t>(BaseAddresses::Registers) + 0xBC,
        PlaybackStart      = static_cast<uint32_t>(BaseAddresses::Registers) + 0xB4,
        PwmDuty            = static_cast<uint32_t>(BaseAddresses::Registers) + 0xD4,
        Rotate             = static_cast<uint32_t>(BaseAddresses::Registers) + 0x58,
        Sound              = static_cast<uint32_t>(BaseAddresses::Registers) + 0x88,
        Swizzle            = static_cast<uint32_t>(BaseAddresses::Registers) + 0x64,
        Tag                = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7C,
        Tagx               = static_cast<uint32_t>(BaseAddresses::Registers) + 0x74,
        Tagy               = static_cast<uint32_t>(BaseAddresses::Registers) + 0x78,
        Vcycle             = static_cast<uint32_t>(BaseAddresses::Registers) + 0x40,
        Voffset            = static_cast<uint32_t>(BaseAddresses::Registers) + 0x44,
        VolSound           = static_cast<uint32_t>(BaseAddresses::Registers) + 0x84,
        VolPb              = static_cast<uint32_t>(BaseAddresses::Registers) + 0x80,
        Vsync0             = static_cast<uint32_t>(BaseAddresses::Registers) + 0x4C,
        Vsync1             = static_cast<uint32_t>(BaseAddresses::Registers) + 0x50,
        Vsize              = static_cast<uint32_t>(BaseAddresses::Registers) + 0x48
    };
    /**
     * @enum TouchScreenEngineRegisters
     * @brief Registers to control the touch screen engine
     */
    enum class TouchScreenEngineRegisters : uint32_t {
        Unknown         = 0,
        TouchConfig     = static_cast<uint32_t>(BaseAddresses::Registers) + 0x168,
        TouchTransformA = static_cast<uint32_t>(BaseAddresses::Registers) + 0x150,
        TouchTransformB = static_cast<uint32_t>(BaseAddresses::Registers) + 0x154,
        TouchTransformC = static_cast<uint32_t>(BaseAddresses::Registers) + 0x158,
        TouchTransformD = static_cast<uint32_t>(BaseAddresses::Registers) + 0x15C,
        TouchTransformE = static_cast<uint32_t>(BaseAddresses::Registers) + 0x160,
        TouchTransformF = static_cast<uint32_t>(BaseAddresses::Registers) + 0x164
    };
    /**
     * @enum ResistiveTouchEngineRegisters
     * @brief Registers to control the resistive touch engine
     */
    enum class ResistiveTouchEngineRegisters : uint32_t {
        Unknown         = 0,
        TouchAdcMode    = static_cast<uint32_t>(BaseAddresses::Registers) + 0x108,
        TouchCharge     = static_cast<uint32_t>(BaseAddresses::Registers) + 0x10C,
        TouchDirectXy   = static_cast<uint32_t>(BaseAddresses::Registers) + 0x18C,
        TouchDirectZ1z2 = static_cast<uint32_t>(BaseAddresses::Registers) + 0x190, 
        TouchMode       = static_cast<uint32_t>(BaseAddresses::Registers) + 0x104,
        TouchOversample = static_cast<uint32_t>(BaseAddresses::Registers) + 0x114,
        TouchRawXy      = static_cast<uint32_t>(BaseAddresses::Registers) + 0x11C,
        TouchRz         = static_cast<uint32_t>(BaseAddresses::Registers) + 0x120,
        TouchRzThresh   = static_cast<uint32_t>(BaseAddresses::Registers) + 0x118,
        TouchScreenXy   = static_cast<uint32_t>(BaseAddresses::Registers) + 0x124,
        TouchSettle     = static_cast<uint32_t>(BaseAddresses::Registers) + 0x110,
        TouchTag        = static_cast<uint32_t>(BaseAddresses::Registers) + 0x12C,
        TouchTagXy      = static_cast<uint32_t>(BaseAddresses::Registers) + 0x128
    };
    /**
     * @enum CapacitiveTouchEngineRegisters
     * @brief Registers to control the capacitive touch engine
     */
    enum class CapcitiveTouchEngineRegisters : uint32_t {
        Unknown        = 0,
        CtouchMode     = static_cast<uint32_t>(BaseAddresses::Registers) + 0x104,
        CtouchExtend   = static_cast<uint32_t>(BaseAddresses::Registers) + 0x108,
        CtouchRawXy    = static_cast<uint32_t>(BaseAddresses::Registers) + 0x11C,
        CtouchTouchXy  = static_cast<uint32_t>(BaseAddresses::Registers) + 0x124,
        CtouchTouch1Xy = static_cast<uint32_t>(BaseAddresses::Registers) + 0x11C,
        CtouchTouch2Xy = static_cast<uint32_t>(BaseAddresses::Registers) + 0x18C,
        CtouchTouch3Xy = static_cast<uint32_t>(BaseAddresses::Registers) + 0x190,
        CtouchTouch4x  = static_cast<uint32_t>(BaseAddresses::Registers) + 0x16C,
        CtouchTouch4y  = static_cast<uint32_t>(BaseAddresses::Registers) + 0x120,
        CtouchTag      = static_cast<uint32_t>(BaseAddresses::Registers) + 0x12C,
        CtouchTag1     = static_cast<uint32_t>(BaseAddresses::Registers) + 0x134,
        CtouchTag2     = static_cast<uint32_t>(BaseAddresses::Registers) + 0x13C,
        CtouchTag3     = static_cast<uint32_t>(BaseAddresses::Registers) + 0x144,
        CtouchTag4     = static_cast<uint32_t>(BaseAddresses::Registers) + 0x14C,
        CtouchTagXy    = static_cast<uint32_t>(BaseAddresses::Registers) + 0x128,
        CtouchTag1Xy   = static_cast<uint32_t>(BaseAddresses::Registers) + 0x130,
        CtouchTag2Xy   = static_cast<uint32_t>(BaseAddresses::Registers) + 0x138,
        CtouchTag3Xy   = static_cast<uint32_t>(BaseAddresses::Registers) + 0x140,
        CtouchTag4Xy   = static_cast<uint32_t>(BaseAddresses::Registers) + 0x148
    };
    /**
     * @enum CoProcessorEngineRegisters
     * @brief Registers for issuing commands to the coprocessor.
     * @details Values are offsets from RAM_REG
     */
    enum class CoprocessorEngineRegisters : uint32_t {
        Unknown                  = 0,
        DisplayListFifoOffset  = static_cast<uint32_t>(BaseAddresses::Registers) + 0x100,   ///< Set the beginning of the display list buffer so the CoPro can execute display list commands
        ReadFifoOffset         = static_cast<uint32_t>(BaseAddresses::Registers) + 0xF8,    ///< Read the coprocessor Fifo location. Use in conjunction with the max size of the buffer to determine the fullness
        WriteFifoOffset = static_cast<uint32_t>(BaseAddresses::Registers) + 0xFC,           ///< After issuing commands, write the total size of all commands issued so the CoPro knows there are new commands to execute.
        FreeSpaceInCommandBuffer = static_cast<uint32_t>(BaseAddresses::Registers) + 0x574, ///< Free space in RAM_CMD
        WriteDataToCommandBuffer = static_cast<uint32_t>(BaseAddresses::Registers) + 0x578  ///< Write to RAM_CMD
    };

    /**
     * @enum MiscellaneousRegisters
     * @brief Miscellaneous registers
     * @details Sect. 3.6 Bt81X programming guide
     */
    enum class MiscellaneousRegisters : uint32_t {
        CpuReset = static_cast<uint32_t>(BaseAddresses::Registers) + 0x20,             ///< Reset one of the audio, touch, or coprocessor engines.
        Macro1 = static_cast<uint32_t>(BaseAddresses::Registers) + 0xDC,               ///< Display list command macro 1
        Macro0 = static_cast<uint32_t>(BaseAddresses::Registers) + 0xD8,               ///< Display list command macro 0.
        PwmDuty = static_cast<uint32_t>(BaseAddresses::Registers) + 0xD4,              ///< These bits define the backlist PWM output duty cycle.
        PwmHz = static_cast<uint32_t>(BaseAddresses::Registers) + 0xD0,                ///< These bits define the backlight PWM output frequency
        InterruptMask = static_cast<uint32_t>(BaseAddresses::Registers) + 0xB0,        ///< Set to mask an interrupt (enable or disable)
        InterruptEnable = static_cast<uint32_t>(BaseAddresses::Registers) + 0xAc,      ///< Set to enable interrupts
        InterruptFlags = static_cast<uint32_t>(BaseAddresses::Registers) + 0xA8,       ///< Read this register to determine which interrupt took place.
        GpioDirection = static_cast<uint32_t>(BaseAddresses::Registers) + 0x90,        ///< Legacy for backward compat. Set the direction of a GPIO
        GpioXDirection = static_cast<uint32_t>(BaseAddresses::Registers) + 0x98,       ///< Set the direction of a GPIO
        GpioParameters = static_cast<uint32_t>(BaseAddresses::Registers) + 0x94,       ///< Legacy for backward compat. Varying bitfields allow you to set GPIO parameters such as drive strength, open drain, etc.
        GpioXParameters = static_cast<uint32_t>(BaseAddresses::Registers) + 0x9C,      ///< Varying bitfields allow you to set GPIO parameters such as drive strength, open drain, etc.
        Frequency = static_cast<uint32_t>(BaseAddresses::Registers) + 0xC,             ///< The main clock frequency which must be updated if an alternate frequency is selected.
        Clock = static_cast<uint32_t>(BaseAddresses::Registers) + 0x8,                 ///< Running total of the number of main clock cycles since reset.
        Frames = static_cast<uint32_t>(BaseAddresses::Registers) + 0x4,                ///< Running total of the number of screen frames.
        Id = static_cast<uint32_t>(BaseAddresses::Registers) + 0x0,                    ///< The value 0x7C is always written at startup. This value means the chip is EVE series and is in working mode afeter bootup
        SpiWidth = static_cast<uint32_t>(BaseAddresses::Registers) + 0x188,            ///< The width of the SPI bus
        AdaptiveFramerate = static_cast<uint32_t>(BaseAddresses::Registers) + 0x57C,   ///< Adaptive framerate
        Underrun = static_cast<uint32_t>(BaseAddresses::Registers) + 0x60C,            ///< Underrun
        AhHycycleMax = static_cast<uint32_t>(BaseAddresses::Registers) + 0x610,        ///< AhHycycleMax
        PixelClockFrequency = static_cast<uint32_t>(BaseAddresses::Registers) + 0x614, ///< Pixel clock frequency
        PixelClock2x = static_cast<uint32_t>(BaseAddresses::Registers) + 0x618,        ///< Pixel clock frequency
        Unknown = static_cast<uint32_t>(BaseAddresses::Registers) + 0x30               ///< Unknown miscellaneous register
    };

    /**
     * @enum SpecialRegisters
     * @brief Special registers
     */
    enum class SpecialRegisters : uint32_t {
        Tracker = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7000,                ///< Tracker
        Tracker1 = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7004,               ///< Tracker1
        Tracker2 = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7008,               ///< Tracker2
        Tracker3 = static_cast<uint32_t>(BaseAddresses::Registers) + 0x700C,               ///< Tracker3
        Tracker4 = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7010,               ///< Tracker4
        MediaFifoRead = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7020,          ///< Media FIFO read
        MediaFifoWrite = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7024,         ///< Media FIFO write
        PlayControl = static_cast<uint32_t>(BaseAddresses::Registers) + 0x714E,            ///< Video playback control to pause, play and exit.
        AnimationActive = static_cast<uint32_t>(BaseAddresses::Registers) + 0x702C,        ///< Each bit indicates the active state of an animation channel
        CoprocessorPatchPointer = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7162,///< Coprocessor patch pointer
        Unknown = static_cast<uint32_t>(BaseAddresses::Registers) + 0x7030                 ///< Unknown special register
    };

    /// @brief The number of bytes in the address space that is readable and writable
    constexpr Bytes AddressSize = 3;
    /// @typedef DisplayListCommand
    /// Value returned by all display list commands
    using DisplayListCommand = uint32_t;

    /**
     * @enum SystemClockFrequency
     * @brief The supported system clock frequencies
     */
    enum class SystemClockFrequency : Hertz {
        Unknown = 0,                  ///< Unknown
        Frequency24MHz = Hertz(24E6), ///< 24MHz
        Frequency36MHz = Hertz(36E6), ///< 36MHz
        Frequency48MHz = Hertz(48E6), ///< 48MHz
        Frequency60MHz = Hertz(60E6), ///< 60MHz
        Frequency72MHz = Hertz(72E6), ///< 72MHz
    };

    /**
     * @enum BitmapPixelFormat
     * @brief Bitmap pixel formats
     * @sa BT81x programming guide, Setc. 4.8 - BITMAP_LAYOUT, Table 13
     */
    enum class BitmapPixelFormat : uint32_t {
        Argb1555,
        L1,
        L4,
        L8,
        Rgb332,
        Argb2,
        Argb4,
        Rgb565,
        Text8x8 = 9,
        TextVga,
        Bargraph,
        Paletted565 = 14,
        Paletted4444,
        Paletted8,
        L2,
        GlFormat = 31,
        Unknown
    };

    /**
     * @enum Options
     * @brief The available options for commands that accept them
     * @sa BT81x Programming Guide, Pg. 109 - Table 29, Sect. 5.9
     */
    enum class Options : uint32_t {
        ThreeDimensional = 0,
        Rgb565 = 0,
        Mono = 1,
        NoDisplayList = 2,
        OneDimensional = 256,
        SignedInteger = 256,
        CenterXCoordinate = 512,
        CenterYCoordinate = 1024,
        Centered = 1536,
        RightJustified = 2048,
        NoBackground = 4096,
        Fill = 8192,
        Format = 4096,
        NoTicks = 8192,
        NoHourAndMinuteHands = 16384,
        NoPointer = 16384,
        NoSeconds = 32768,
        NoTear = 4,
        FullScreen = 8,
        MediaFifo = 16,
        Sound = 32,
        Dither = 256,
        NoOptions,
        Unknown
    };
    
    /**
     * @enum Font
     * @brief The font to use
     */
    enum class Font : uint32_t {
        Unknown = 0, ///< Unknown font
        Font0 = 16,  ///< Font 0
        Font1,       ///< Font 1      
        Font2,       ///< Font 2
        Font3,       ///< Font 3
        Font4,       ///< Font 4
        Font5,       ///< Font 5
        Font6,       ///< Font 6
        Font7,       ///< Font 7
        Font8,       ///< Font 8
        Font9,       ///< Font 9
        Font10,      ///< Font 10
        Font11,      ///< Font 11
        Font12,      ///< Font 12
        Font13,      ///< Font 13
        Font14,      ///< Font 14
        Font15,      ///< Font 15
        Font16,      ///< Font 16
        Font17,      ///< Font 17
        Font18,      ///< Font 18
    };

    /**
     * @namespace DisplayListCommands
     * @brief Functions to create display list commands
     * @details The immediately invoked lambdas are an improvement over macro-based implementations because they are type-safe.
     */
    namespace DisplayListCommands {
        inline constexpr DisplayListCommand AlphaTestFunction(const uint32_t function, const uint32_t referenceValue) {
            return (9UL << 24) |
                   ((function & 7UL) << 8) |
                   ((referenceValue & 255UL) << 0);
        }

        /**
         * @enum GraphicsPrimitive
         * @brief Graphics primitives
         */
        enum class GraphicsPrimitive : uint8_t {
            Unknown = 0,
            Bitmaps,
            Points,
            Lines,
            LineStrip,
            EdgeStripRight,
            EdgeStripLeft,
            EdgeStripAbove,
            EdgeStripBelow,
            Rectangles
        };
        /**
         * @brief Start a new graphics primitive
         * @param[in] primitive The primitive to start
         */
        inline constexpr DisplayListCommand BeginGraphicsPrimitive(const GraphicsPrimitive primitive) {
            return (0x1F << 24) | (static_cast<uint8_t>(primitive) & 0xF);
        }

        inline constexpr DisplayListCommand BitmapExtendedFormat(const uint32_t format) {
            return (46UL << 24) | (format & 65535UL);
        }

        inline constexpr DisplayListCommand BitmapHandle(const uint32_t handle) {
            return (5UL << 24) | (handle & 0x1F);
        }

        inline constexpr DisplayListCommand BitmapLayout(const BitmapPixelFormat format, const uint32_t lineStride, const uint32_t height) {
            return (7UL << 24) |
                   ((static_cast<uint32_t>(format) & 0x1F) << 19) |
                   ((lineStride & 0x3FF) << 9) |
                   ((height & 0x1FF) << 0);
        }

        inline constexpr DisplayListCommand BitmapLinestrideAndHeight(const uint32_t lineStride, const uint32_t height) {
            return (40UL << 24) |
                   ((lineStride & 3UL) << 2) |
                   ((height & 3UL) << 0);
        }

        /**
         * @enum BitmapFilteringMode
         * @brief Bitmap filtering modes
         */
        enum class BitmapFilteringMode : uint8_t {
            Nearest = 0,  ///< Nearest neighbor
            Bilinear = 1, ///< Bilinear
            Unknown = 2   ///< Unknown
        };
        /**
         * @enum BitmapWrapMode
         * @brief Bitmap wrap modes
         */
        enum class BitmapWrapMode : uint8_t {
            Border = 0, ///< Border
            Repeat = 1, ///< Repeat
            Unknown = 2 ///< Unknown
        };
        /**
         * @brief Specifiy the screen drawing of bitmaps for the current display list.
         * @param[in] filter The filtering mode to use
         * @param[in] wrapX True if the bitmap should wrap horizontally
         * @param[in] wrapY True if the bitmap should wrap vertically
         * @param[in] width The width of the bitmap
         * @param[in] height The height of the bitmap
         */
        inline constexpr DisplayListCommand BitmapSize(const BitmapFilteringMode filter, const BitmapWrapMode wrapX, const BitmapWrapMode wrapY, const Area &area) {
            return (8UL << 24) |
                   ((static_cast<uint8_t>(filter) & 1UL) << 20) |
                   ((static_cast<uint8_t>(wrapX) & 1UL) << 19) |
                   ((static_cast<uint8_t>(wrapY) & 1UL) << 18) |
                   ((area.width & 0x1FF) << 9) |
                   ((area.height & 0x1FF));
        }

        inline constexpr DisplayListCommand BitmapExtendedWidthAndHeight(const uint32_t width, const uint32_t height) {
            return (41UL << 24) |
                   ((width & 3UL) << 2) |
                   ((height & 3UL));
        }
        /**
         * @brief Pg. 65, BT81X Programming Guide
         * @returns 0 if the address is invalid.
         */
        inline constexpr DisplayListCommand BitmapSourceAddress(const uint32_t address) {
            constexpr uint32_t ramGStart = static_cast<uint32_t>(BaseAddresses::GeneralPurposeGraphicsRam);
            constexpr uint32_t ramGEnd = ramGStart + static_cast<uint32_t>(AddressSpace::GeneralPurposeGraphicsRam);
            constexpr uint32_t flashStart = static_cast<uint32_t>(BaseAddresses::Flash);
            constexpr uint32_t flashEnd = flashStart + static_cast<uint32_t>(AddressSpace::Flash);

            constexpr uint32_t setTop4BitsToOne = 0x1FFFFFFF;
            constexpr uint32_t setAddressToBits0To22 = 0x07FFFFFF;

            if (address >= ramGStart && address < ramGEnd) {
                constexpr uint32_t clearBit23 = ~(1 << 23);
                return (setTop4BitsToOne | ((address & setAddressToBits0To22))) & clearBit23;
            }
            else if (address >= flashStart && address < flashEnd) {
                constexpr uint32_t setBit23 = (1 << 23);
                return (setTop4BitsToOne | ((address*32) & setAddressToBits0To22)) | setBit23;
            }
            else {
                return 0;
            }
        }
        /**
         * Set the source for the red, green, blue and alpha channels of a bitmap.
         */
        inline constexpr DisplayListCommand BitmapSwizzle(const uint32_t red, const uint32_t green, const uint32_t blue, const uint32_t alpha) {
            return (47UL << 24) |
                   ((red & 7UL) << 9) |
                   ((green & 7UL) << 6) |
                   ((blue & 7UL) << 3) |
                   ((alpha & 7UL));
        }
        /**
         * Specify the A coefficient of the bitmap transform matrix.
         */
        inline constexpr DisplayListCommand BitmapTransformA(const uint32_t precision, const int32_t component) {
            return (21UL << 24) |
                   ((precision & 1UL) << 17) |
                   (((uint32_t)component & 131071UL));
        }
        /**
         * Specify the B coefficient of the bitmap transform matrix.
         */
        inline constexpr DisplayListCommand BitmapTransformB(const uint32_t precision, const int32_t component) {
            return (22UL << 24) |
                   ((precision & 1UL) << 17) |
                   (((uint32_t)component & 131071UL) << 0);
        }
        /**
         * Specify the C coefficient of the bitmap transform matrix.
         */
        inline constexpr DisplayListCommand BitmapTransformC(const int32_t component) {
            return (23UL << 24) | (((uint32_t)component & 16777215UL));
        }
        /**
         * Specify the D coefficient of the bitmap transform matrix.
         */
        inline constexpr DisplayListCommand BitmapTransformD(const uint32_t precision, const int32_t component) {
            return (24UL << 24) |
                   ((precision & 1UL) << 17) |
                   (((uint32_t)component & 131071UL));
        }
        /**
         * Specify the E coefficient of the bitmap transform matrix.
         */
        inline constexpr DisplayListCommand BitmapTransformE(const uint32_t precision, const int32_t component) {
            return (25UL << 24) |
                   ((precision & 1UL) << 17) |
                   (((uint32_t)component & 131071UL));
        }
        /**
         * Specify the F coefficient of the bitmap transform matrix.
         */
        inline constexpr DisplayListCommand BitmapTransformF(const int32_t component) {
            return (26UL << 24) | (((uint32_t)component & 16777215UL));
        }

        enum class BlendingFactor : uint8_t {
            Zero                = 0,
            One                 = 1,
            SourceAlpha         = 2,
            DestinationAlpha    = 3,
            OneMinusSourceAlpha = 4,
            OneMinusDestAlpha   = 5,
            Unknown             = 6
        };
        inline constexpr DisplayListCommand Blend(const BlendingFactor sourceBlendingFactor, const BlendingFactor destinationBlendingFactor) {
            return (11UL << 24) |
                   ((static_cast<uint8_t>(sourceBlendingFactor) & 7UL) << 3) |
                   ((static_cast<uint8_t>(destinationBlendingFactor) & 7UL));
        }
        /**
         * @brief Execute a sequence of commands at another location in the display list
         */
        inline constexpr DisplayListCommand Call(const uint32_t dest) {
            return (29UL << 24) | (dest & 65535UL);
        }
        /**
         * @brief Specify the bitmap cell number for the Vertex2f command
         */
        inline constexpr DisplayListCommand CellNumberForVertex2f(const uint32_t cell) {
            return (6UL << 24) | (cell & 127UL);
        }

        inline constexpr DisplayListCommand Clear(const uint8_t clearColourBuffer, const uint8_t clearStencilBuffer, const uint8_t clearTagBuffer) {
            return (0x26 << 24) |
                   ((clearColourBuffer & 1UL) << 2) |
                   ((clearStencilBuffer & 1UL) << 1) |
                   ((clearTagBuffer & 1UL));
        }

        inline constexpr DisplayListCommand ClearColorA(const uint32_t alpha) {
            return (15UL << 24) | (alpha & 255UL);
        }

        inline constexpr DisplayListCommand ClearColorRgb(const HexCodeColour colour) {
            return (0x2 << 24) | (colour & 0x00FFFFFF);
        }

        inline constexpr DisplayListCommand ClearStencil(const uint32_t s) {
            return (17UL << 24) | (s & 255UL);
        }

        inline constexpr DisplayListCommand ClearTag(const uint32_t s) {
            return (18UL << 24) | (s & 255UL);
        }

        inline constexpr DisplayListCommand ColorA(const uint32_t alpha) {
            return (16UL << 24) | (alpha & 255UL);
        }

        inline constexpr DisplayListCommand ColorMask(const uint32_t r, const uint32_t g, const uint32_t b, const uint32_t a) {
            return (32UL << 24) |
                   ((r & 1UL) << 3) |
                   ((g & 1UL) << 2) |
                   ((b & 1UL) << 1) |
                   ((a & 1UL));
        }

        inline constexpr DisplayListCommand ColorRgb(const HexCodeColour colour) {
            return (4UL << 24) | (colour & 0x00FFFFFF);
        }

        inline constexpr DisplayListCommand Display() {
            return 0;
        }

        inline constexpr DisplayListCommand EndGraphicsPrimitive() {
            return (33UL << 24);
        }

        inline constexpr DisplayListCommand IntFrr() {
            return (48UL << 24);
        }

        inline constexpr DisplayListCommand Jump(const uint32_t dest) {
            return (30UL << 24) | (dest & 0xFFFF);
        }

        inline constexpr DisplayListCommand LineWidth(const uint32_t width) {
            return (14UL << 24) | (width & 0xFFF);
        }

        inline constexpr DisplayListCommand Macro(const uint32_t m) {
            return (37UL << 24) | (m & 1UL);
        }

        inline constexpr DisplayListCommand Nop() {
            return (45UL << 24);
        }

        inline constexpr DisplayListCommand PaletteSource(const uint32_t addr) {
            return (42UL << 24) | (addr & 4194303UL);
        }

        inline constexpr DisplayListCommand PointSize(const uint32_t size) {
            return (13UL << 24) | (size & 8191UL);
        }

        inline constexpr DisplayListCommand RestoreContext() {
            return (35UL << 24);
        }

        inline constexpr DisplayListCommand ReturnFromCall() {
            return (36UL << 24);
        }

        inline constexpr DisplayListCommand SaveContext() {
            return (34UL << 24);
        }

        inline constexpr DisplayListCommand ScissorSize(const uint32_t width, const uint32_t height) {
            return (28UL << 24) |
                   ((width & 4095UL) << 12) |
                   ((height & 4095UL));
        }

        inline constexpr DisplayListCommand ScissorXy(const uint32_t x, const uint32_t y) {
            return (27UL << 24) |
                   ((x & 2047UL) << 11) |
                   ((y & 2047UL));
        }

        inline constexpr DisplayListCommand StencilFunc(const uint32_t func, const uint32_t ref, const uint32_t mask) {
            return (10UL << 24) |
                   ((func & 7UL) << 16) |
                   ((ref & 255UL) << 8) |
                   ((mask & 255UL));
        }

        inline constexpr DisplayListCommand StencilMask(const uint32_t mask) {
            return (19UL << 24) | (mask & 255UL);
        }

        inline constexpr DisplayListCommand StencilOp(const uint32_t sFail, const uint32_t sPass) {
            return (12UL << 24) |
                   ((sFail & 7UL) << 3) |
                   ((sPass & 7UL));
        }

        inline constexpr DisplayListCommand GraphicsObjectTag(const uint32_t s) {
            return (3UL << 24) | (s & 255UL);
        }

        inline constexpr DisplayListCommand GraphicsObjectTagMask(const uint32_t mask) {
            return (20UL << 24) | (mask & 1UL);
        }

        inline constexpr DisplayListCommand Vertex2F(const int32_t x, const int32_t y) {
            return (1UL << 30) |
                   (((uint32_t)x & 32767UL) << 15) |
                   (((uint32_t)y & 32767UL));
        }

        inline constexpr DisplayListCommand Vertex2II(const uint32_t x, const uint32_t y, const uint32_t handle, const uint32_t cell) {
            return (2UL << 30) |
                   ((x & 511UL) << 21) |
                   ((y & 511UL) << 12) |
                   ((handle & 31UL) << 7) |
                   ((cell & 127UL));
        }

        inline constexpr DisplayListCommand VertexFormat(const uint32_t frac) {
            return (39UL << 24) | (frac & 7UL);
        }

        inline constexpr DisplayListCommand VertexTranslateX(const int32_t x) {
            return (43UL << 24) | (((uint32_t)x & 131071UL));
        }

        inline constexpr DisplayListCommand VertexTranslateY(const int32_t y) {
            return (44UL << 24) | (((uint32_t)y & 131071UL));
        }
    }
}