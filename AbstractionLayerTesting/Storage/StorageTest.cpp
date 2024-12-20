//Modules
#include "StorageModule.hpp"
#include "FileModule.hpp"
#include "OperatingSystemModule.hpp"
//Applications
#include "Log.hpp"


static const char TAG[] = "storageTest";

static void *startStorageThread(void *arg) {
    while (true) {
        OperatingSystem::Instance().delay(1000);
        Storage::Instance().mainLoop();
    }
    return nullptr;
}

static int testReadWrite() {
    ErrorType error = ErrorType::Success;
    std::string filename("/testFile");

    File file(Storage::Instance());
    assert(ErrorType::Success == (error = file.open(filename, OpenMode::ReadWriteAppend)));

    const std::string writeString("Hello World!");
    std::string readString(writeString.size(), 0);
    Bytes written = 0;

    assert(ErrorType::Success == (error = file.writeBlocking(0, writeString)));

    assert(ErrorType::Success == (error = file.readBlocking(0, readString)));

    if (0 == readString.compare(writeString)) {
        assert(ErrorType::Success == file.close());
        //ErrorType::FileNotFound is ok. It's not neccessary to close before removing since remove closes the file before it removes
        //but this is a common practice among programmers.
        assert(ErrorType::Failure != file.remove());

        return EXIT_SUCCESS;
    }
    else {
        PLT_LOGE(TAG, "Error reading from file %u. data: %s", (uint8_t)error, readString.c_str());
    }

    assert(false);
}

static int testNonBlockingReadWrite() {
    ErrorType error = ErrorType::Success;
    std::string filename("/testFile");

    File file(Storage::Instance());
    if (ErrorType::Success != (error = file.open(filename, OpenMode::ReadWriteTruncate))) {
        PLT_LOGE(TAG, "Error opening file %u", (uint8_t)error);
        assert(false);
    }

    const auto writeString = std::make_shared<std::string>("Hello World!");
    auto buffer = std::make_shared<std::string>(writeString->size(), 0);
    Bytes written = 0;
    const Bytes toWrite = writeString->size();
    ErrorType readError = ErrorType::Failure;
    ErrorType writeError = ErrorType::Failure;

    auto readCallback = [&readError, writeString](const ErrorType error, std::shared_ptr<std::string> buffer) {
        if (0 == buffer->compare(*(writeString.get()))) {
            readError = error;
        }
        else {
            PLT_LOGE(TAG, "Error reading from file Error: %u, data: %s", error, buffer->c_str());
            readError = error;
            assert(false);
        }
    };

    auto writeCallback = [&writeError, toWrite](const ErrorType error, const Bytes written) {
        if (ErrorType::Success == error && written == toWrite) {
            writeError = error;
        }
        else {
            PLT_LOGE(TAG, "Error writing to file Error: %u, written: %u, toWrite: %u", error, written, toWrite);
            writeError = error;
            assert(false);
        }
    };

    file.writeNonBlocking(writeString, writeCallback);
    file.readNonBlocking((FileOffset)0, buffer, readCallback);

    //Wait for the thread to process the read and write events.
    while (ErrorType::Success != readError && ErrorType::Success != writeError) {
        OperatingSystem::Instance().delay(10);
    }

    assert(ErrorType::Success == (error = file.close()));
    assert(ErrorType::Failure != (error = file.remove()));

    return EXIT_SUCCESS;
}

static int useWithoutOpening() {
    ErrorType error = ErrorType::Success;
    std::string filename("/testFile");
    Bytes written = 0;

    File file(Storage::Instance());

    //Write without opening
    error = file.writeBlocking(0, filename.data());
    assert(ErrorType::Success != error);

    //Close without opening
    error = file.close();
    assert(ErrorType::PrerequisitesNotMet == error);

    //File doesn't exist, so let's try removing it.
    error = file.remove();
    assert(ErrorType::PrerequisitesNotMet == error);

    //Try reading the non-existent file.
    error = file.readBlocking(0, filename);
    if (ErrorType::PrerequisitesNotMet != error) {
        PLT_LOGE(TAG, "File should not be readable %u", (uint8_t)error);
        assert(false);
    }

    if (error != ErrorType::Success) {
      return EXIT_SUCCESS;
    }
    else {
        assert(false);
    }
}

static int openFlags() {
    ErrorType error = ErrorType::Success;
    std::string filename("/testFile");
    std::string writeString("openFlagsTest");
    std::string readString(writeString.size(), 0);
    Bytes written = 0;

    File file(Storage::Instance());
    //Open the file as read-only.
    assert(ErrorType::Success == file.open(filename, OpenMode::ReadOnly));
    //Open the file as write-only. Should fail because it's already open.
    assert(ErrorType::PrerequisitesNotMet == file.open(filename, OpenMode::WriteOnlyAppend));
    //Now try to write to it. Prereqs for writing not met since the open mode doesn't allow writing.
    assert(ErrorType::PrerequisitesNotMet == file.writeBlocking(0, writeString));
    //Close the file
    assert(ErrorType::Success == file.close());

    assert(ErrorType::Success == file.open(filename, OpenMode::WriteOnlyAppend));
    assert(ErrorType::Success == file.writeBlocking(0, writeString));
    //Can't read a write-only file
    assert(ErrorType::PrerequisitesNotMet == file.readBlocking(0, readString));
    assert(ErrorType::Success == file.remove());

    return EXIT_SUCCESS;
}

static int runAllTests() {
    std::vector<std::function<int(void)>> tests = {
        //testReadWrite,
        //testNonBlockingReadWrite,
        //useWithoutOpening,
        openFlags
    };

    for (auto test : tests) {
        if (EXIT_SUCCESS != test()) {
            assert(false);
        }
    }

    return EXIT_SUCCESS;
}

int main() {

    OperatingSystem::Init();
    Logger::Init();

    Storage::Init(std::string("testStorage"));
    if (ErrorType::Success != Storage::Instance().initStorage()) {
        PLT_LOGE(TAG, "initStorage failed");
        assert(false);
    }

    Bytes ramSize;
    Storage::Instance().maxRamSize(ramSize);
    Bytes availableRam;
    Storage::Instance().availableRam(availableRam);

    constexpr uint16_t kilobyte = 1024;
    Id storageId;

    OperatingSystem::Instance().createThread(OperatingSystemConfig::Priority::Low, "storageThread", nullptr, 4*kilobyte, startStorageThread, storageId);

    int result = runAllTests();

    return result;
}
