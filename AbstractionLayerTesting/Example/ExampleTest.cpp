//C++
#include <vector>
#include <functional>
//Modules
#include "Log.hpp"
#include "OperatingSystemModule.hpp"

static const char TAG[] = "exampleTest";

static int exampleTest1() {
    const bool testWillPass = true;

    if (testWillPass) {
        PLT_LOGI(TAG, "exampleTest1 passed");
        return EXIT_SUCCESS;
    }
    else {
        PLT_LOGE(TAG, "exampleTest1 failed");
        return EXIT_FAILURE;
    }
}

static int runAllTests() {
    std::vector<std::function<int(void)>> tests = {
        exampleTest1
    };

    for (auto test : tests) {
        if (EXIT_SUCCESS != test()) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int main() {

    OperatingSystem::Init();
    Logger::Init();

    int result = runAllTests();
    toPlatformError(result);

    return result;
}