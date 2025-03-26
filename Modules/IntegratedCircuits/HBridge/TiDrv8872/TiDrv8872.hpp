#include "HBridgeAbstraction.hpp"

/**
 * @class TiDrv8872
 * @brief HBridge by Texas Instruments
 * @sa https://www.ti.com/lit/ds/symlink/drv8872-q1.pdf?ts=1740601343172&ref_url=https%253A%252F%252Fwww.mouser.it%252F
 */
class TiDrv8872 : public HBridgeAbstraction {
    public:
    /// @brief  Constructor
    TiDrv8872() = default;
    /// @brief Destructor
    ~TiDrv8872() = default;

    ErrorType driveForward() override;
    ErrorType driveBackward() override;
    ErrorType coast() override;
    ErrorType brake() override;
};