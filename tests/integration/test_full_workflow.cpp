#include <gtest/gtest.h>
#include <QCoreApplication>
#include "../../src/use_case/device_management_use_case.h"
#include "../../src/infrastructure/persistence/device_repository_impl.h"
#include "../../src/core/entities/device.h"

using namespace use_case;
using namespace infrastructure::persistence;
using namespace core::entities;

class FullWorkflowTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 1;
        char* argv[] = {(char*)"test"};
        app = std::make_unique<QCoreApplication>(argc, argv);
        
        repository = std::make_shared<DeviceRepositoryImpl>();
        useCase = std::make_unique<DeviceManagementUseCase>(repository);
    }

    void TearDown() override {
        useCase.reset();
        repository.reset();
        app.reset();
    }

    std::unique_ptr<QCoreApplication> app;
    std::shared_ptr<DeviceRepositoryImpl> repository;
    std::unique_ptr<DeviceManagementUseCase> useCase;
};

TEST_F(FullWorkflowTest, AddAndRetrieveDevice_ShouldWork) {
    // Create a device
    auto device = std::make_shared<Device>("test_device_1", "Test Device");
    device->setModel("Pixel 6");
    device->setManufacturer("Google");
    device->setStatus(DeviceStatus::Authorized);
    
    // Add device through use case
    bool addResult = useCase->addDevice(device);
    EXPECT_TRUE(addResult);
    
    // Retrieve device by ID
    auto retrievedDevice = useCase->getDeviceById("test_device_1");
    EXPECT_NE(retrievedDevice, nullptr);
    EXPECT_EQ(retrievedDevice->id(), "test_device_1");
    EXPECT_EQ(retrievedDevice->name(), "Test Device");
    EXPECT_EQ(retrievedDevice->model(), "Pixel 6");
    EXPECT_EQ(retrievedDevice->manufacturer(), "Google");
    EXPECT_EQ(retrievedDevice->status(), DeviceStatus::Authorized);
}

TEST_F(FullWorkflowTest, ConnectAndDisconnectDevice_ShouldWork) {
    // Add a device
    auto device = std::make_shared<Device>("test_device_2", "Test Device 2");
    useCase->addDevice(device);
    
    // Initially not connected
    EXPECT_FALSE(useCase->isDeviceConnected("test_device_2"));
    
    // Connect device
    bool connectResult = useCase->connectToDevice("test_device_2");
    EXPECT_TRUE(connectResult);
    EXPECT_TRUE(useCase->isDeviceConnected("test_device_2"));
    
    // Disconnect device
    bool disconnectResult = useCase->disconnectFromDevice("test_device_2");
    EXPECT_TRUE(disconnectResult);
    EXPECT_FALSE(useCase->isDeviceConnected("test_device_2"));
}

TEST_F(FullWorkflowTest, RemoveDevice_ShouldWork) {
    // Add a device
    auto device = std::make_shared<Device>("test_device_3", "Test Device 3");
    useCase->addDevice(device);
    
    // Verify device exists
    auto retrievedDevice = useCase->getDeviceById("test_device_3");
    EXPECT_NE(retrievedDevice, nullptr);
    
    // Remove device
    bool removeResult = useCase->removeDevice("test_device_3");
    EXPECT_TRUE(removeResult);
    
    // Verify device no longer exists
    auto removedDevice = useCase->getDeviceById("test_device_3");
    EXPECT_EQ(removedDevice, nullptr);
}

TEST_F(FullWorkflowTest, GetAllDevices_ShouldReturnAllDevices) {
    // Add multiple devices
    auto device1 = std::make_shared<Device>("device_1", "Device 1");
    auto device2 = std::make_shared<Device>("device_2", "Device 2");
    auto device3 = std::make_shared<Device>("device_3", "Device 3");
    
    useCase->addDevice(device1);
    useCase->addDevice(device2);
    useCase->addDevice(device3);
    
    // Get all devices
    auto allDevices = useCase->getAllDevices();
    EXPECT_EQ(allDevices.size(), 3);
    
    // Verify all devices are present
    bool found1 = false, found2 = false, found3 = false;
    for (const auto& device : allDevices) {
        if (device->id() == "device_1") found1 = true;
        if (device->id() == "device_2") found2 = true;
        if (device->id() == "device_3") found3 = true;
    }
    
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
    EXPECT_TRUE(found3);
}

TEST_F(FullWorkflowTest, NonExistentDevice_ShouldReturnNull) {
    auto device = useCase->getDeviceById("non_existent_device");
    EXPECT_EQ(device, nullptr);
}

TEST_F(FullWorkflowTest, NonExistentDeviceConnection_ShouldReturnFalse) {
    bool connectResult = useCase->connectToDevice("non_existent_device");
    EXPECT_FALSE(connectResult);
    
    bool disconnectResult = useCase->disconnectFromDevice("non_existent_device");
    EXPECT_FALSE(disconnectResult);
    
    bool isConnected = useCase->isDeviceConnected("non_existent_device");
    EXPECT_FALSE(isConnected);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 