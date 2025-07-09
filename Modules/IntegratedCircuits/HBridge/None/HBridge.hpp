#include "HBridgeAbstraction.hpp"

class HBridge final : public HBridgeAbstraction {
    public:
    HBridge() = default;

    ErrorType driveForward() override;
    ErrorType driveBackward() override;
    ErrorType coast() override;
    ErrorType brake() override;
};