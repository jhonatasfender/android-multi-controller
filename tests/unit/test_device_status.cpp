#include <gtest/gtest.h>
#include "../../src/core/entities/device_status.h"

using namespace core::entities;

class DeviceStatusTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup if needed
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

TEST_F(DeviceStatusTest, EnumValues_ShouldBeCorrect) {
    EXPECT_EQ(static_cast<int>(DeviceStatus::Unknown), 0);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Offline), 1);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Bootloader), 2);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Download), 3);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Recovery), 4);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Unauthorized), 5);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Authorized), 6);
}

TEST_F(DeviceStatusTest, EnumConversion_ShouldWork) {
    DeviceStatus status1 = static_cast<DeviceStatus>(0);
    EXPECT_EQ(status1, DeviceStatus::Unknown);
    
    DeviceStatus status2 = static_cast<DeviceStatus>(6);
    EXPECT_EQ(status2, DeviceStatus::Authorized);
}

TEST_F(DeviceStatusTest, EnumComparison_ShouldWork) {
    EXPECT_TRUE(DeviceStatus::Unknown < DeviceStatus::Authorized);
    EXPECT_TRUE(DeviceStatus::Offline < DeviceStatus::Bootloader);
    EXPECT_FALSE(DeviceStatus::Authorized < DeviceStatus::Unauthorized);
}

TEST_F(DeviceStatusTest, EnumAssignment_ShouldWork) {
    DeviceStatus status = DeviceStatus::Unknown;
    EXPECT_EQ(status, DeviceStatus::Unknown);
    
    status = DeviceStatus::Authorized;
    EXPECT_EQ(status, DeviceStatus::Authorized);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 