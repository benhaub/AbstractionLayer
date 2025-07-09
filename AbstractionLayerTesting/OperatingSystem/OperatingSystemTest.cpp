//C++
#include <vector>
#include <functional>
//Modules
#include "Log.hpp"
#include "OperatingSystemModule.hpp"

static const char TAG[] = "operatingSystem";
bool threadWasCreated = false;

#ifdef __cplusplus
extern "C" {
#endif

static void *testThreadStartFunction(void *arg) {
    threadWasCreated = true;

    return nullptr;
}

static void *testSemaphoreStartFunction(void *arg) {
    std::string *semaphoreName = reinterpret_cast<std::string *>(arg);

    OperatingSystem::Instance().incrementSemaphore(*semaphoreName);

    return nullptr;
}

#ifdef __cplusplus
}
#endif

//If the thread is created successfully. It should run the start function and set the variable.
//Note that some system will return from thread_creation functions first before calling the thread
//while others will call the start thread before the thread create functions returns. On ESP for example,
//The start function is not called until after the createThread returns. On Linux and MAC, it's called before
//so in this test you could comment out threadJoin and this test would still pass.
static int createThreadTest() {
    OperatingSystemConfig::Priority priority = OperatingSystemConfig::Priority::Normal;
    std::string name("testThread");
    Bytes stackSize = 4096;
    Id threadId;
    ErrorType error;

    error = OperatingSystem::Instance().createThread(priority, name, nullptr, stackSize, testThreadStartFunction, threadId);
    assert(ErrorType::Success == error);
    assert(ErrorType::Success == OperatingSystem::Instance().isDeleted(name));

    OperatingSystem::Instance().delay(Milliseconds(1000));

    assert(true == threadWasCreated);

    OperatingSystem::Instance().deleteThread(name);

    assert(ErrorType::NoData == OperatingSystem::Instance().isDeleted(name));

    return EXIT_SUCCESS;
}

static int semaphoreTest() {
    ErrorType error;

    Count max = 1;
    Count initial = 0;
    std::string semaphoreName("testSemaphore");
    error = OperatingSystem::Instance().createSemaphore(max, initial, semaphoreName);
    assert(ErrorType::Success == error);
    
    OperatingSystemConfig::Priority priority = OperatingSystemConfig::Priority::Normal;
    std::string threadName("testThread");
    Bytes stackSize = 4096;
    Id threadId;
    error = OperatingSystem::Instance().createThread(priority, threadName, &semaphoreName, stackSize, testSemaphoreStartFunction, threadId);
    assert(ErrorType::Success == error);

    error = OperatingSystem::Instance().waitSemaphore(semaphoreName, 1000);
    assert(ErrorType::Success == error);

    return EXIT_SUCCESS;
}

static int runAllTests() {
    std::vector<std::function<int(void)>> tests = {
        createThreadTest,
        semaphoreTest
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
    Percent idlePercent;

    OperatingSystem::Instance().idlePercentage(idlePercent);

    int result = runAllTests();
    toPlatformError(result);

    return result;
}
