#include "HBridgeAbstraction.hpp"

class HBridge : public HBridgeAbstraction {
    public:
    HBridge() = default;
    ~HBridge() = default;

    ErrorType driveForward() override;
    ErrorType driveBackward() override;
    ErrorType coast() override;
    ErrorType brake() override;
};