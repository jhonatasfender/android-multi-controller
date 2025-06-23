#include <gtest/gtest.h>
#include "../../src/core/entities/device.h"

using namespace core::entities;

class DeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        device = std::make_unique<Device>("test_id", "Test Device");
    }

    void TearDown() override {
        device.reset();
    }

    std::unique_ptr<Device> device;
};

TEST_F(DeviceTest, Constructor_ShouldSetIdAndName) {
    EXPECT_EQ(device->id(), "test_id");
    EXPECT_EQ(device->name(), "Test Device");
}

TEST_F(DeviceTest, DefaultValues_ShouldBeCorrect) {
    EXPECT_EQ(device->model(), "");
    EXPECT_EQ(device->manufacturer(), "");
    EXPECT_EQ(device->status(), DeviceStatus::Unknown);
    EXPECT_FALSE(device->connected());
    EXPECT_EQ(device->ipAddress(), "");
    EXPECT_EQ(device->port(), 0);
}

TEST_F(DeviceTest, SetModel_ShouldUpdateModel) {
    device->setModel("Pixel 6");
    EXPECT_EQ(device->model(), "Pixel 6");
}

TEST_F(DeviceTest, SetManufacturer_ShouldUpdateManufacturer) {
    device->setManufacturer("Google");
    EXPECT_EQ(device->manufacturer(), "Google");
}

TEST_F(DeviceTest, SetStatus_ShouldUpdateStatus) {
    device->setStatus(DeviceStatus::Authorized);
    EXPECT_EQ(device->status(), DeviceStatus::Authorized);
}

TEST_F(DeviceTest, SetConnected_ShouldUpdateConnected) {
    device->setConnected(true);
    EXPECT_TRUE(device->connected());
}

TEST_F(DeviceTest, SetIpAddress_ShouldUpdateIpAddress) {
    device->setIpAddress("192.168.1.100");
    EXPECT_EQ(device->ipAddress(), "192.168.1.100");
}

TEST_F(DeviceTest, SetPort_ShouldUpdatePort) {
    device->setPort(5555);
    EXPECT_EQ(device->port(), 5555);
}

TEST_F(DeviceTest, MultiplePropertyChanges_ShouldWorkCorrectly) {
    device->setModel("Galaxy S21");
    device->setManufacturer("Samsung");
    device->setStatus(DeviceStatus::Authorized);
    device->setConnected(true);
    device->setIpAddress("192.168.1.101");
    device->setPort(5556);

    EXPECT_EQ(device->model(), "Galaxy S21");
    EXPECT_EQ(device->manufacturer(), "Samsung");
    EXPECT_EQ(device->status(), DeviceStatus::Authorized);
    EXPECT_TRUE(device->connected());
    EXPECT_EQ(device->ipAddress(), "192.168.1.101");
    EXPECT_EQ(device->port(), 5556);
}

TEST_F(DeviceTest, DeviceStatusEnum_ShouldHaveCorrectValues) {
    EXPECT_EQ(static_cast<int>(DeviceStatus::Unknown), 0);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Offline), 1);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Bootloader), 2);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Download), 3);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Recovery), 4);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Unauthorized), 5);
    EXPECT_EQ(static_cast<int>(DeviceStatus::Authorized), 6);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 