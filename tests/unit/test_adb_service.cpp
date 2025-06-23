#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../src/infrastructure/adb/adb_service.h"

using namespace infrastructure::adb;
using namespace testing;

class AdbServiceTest : public Test {
protected:
    void SetUp() override {
        adbService = std::make_unique<AdbService>();
    }

    void TearDown() override {
        adbService.reset();
    }

    std::unique_ptr<AdbService> adbService;
};

TEST_F(AdbServiceTest, Constructor_ShouldInitializeCorrectly) {
    EXPECT_NE(adbService, nullptr);
}

TEST_F(AdbServiceTest, GetConnectedDevices_ShouldReturnEmptyListInitially) {
    auto devices = adbService->getConnectedDevices();
    EXPECT_TRUE(devices.isEmpty());
}

TEST_F(AdbServiceTest, IsDeviceConnected_ShouldReturnFalseForNonExistentDevice) {
    bool connected = adbService->isDeviceConnected("non_existent_device");
    EXPECT_FALSE(connected);
}

TEST_F(AdbServiceTest, ExecuteCommand_ShouldReturnFalseForNonExistentDevice) {
    bool result = adbService->executeCommand("non_existent_device", "ls");
    EXPECT_FALSE(result);
}

TEST_F(AdbServiceTest, StartServer_ShouldNotThrow) {
    EXPECT_NO_THROW(adbService->startServer());
}

TEST_F(AdbServiceTest, StopServer_ShouldNotThrow) {
    EXPECT_NO_THROW(adbService->stopServer());
}

TEST_F(AdbServiceTest, GetServerStatus_ShouldReturnValidStatus) {
    auto status = adbService->getServerStatus();
    // Status should be one of the valid enum values
    EXPECT_TRUE(status == AdbService::ServerStatus::Running || 
                status == AdbService::ServerStatus::Stopped || 
                status == AdbService::ServerStatus::Error);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 