#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../src/use_case/device_management_use_case.h"
#include "../../src/core/entities/device.h"

using namespace use_case;
using namespace testing;

class MockDeviceRepository : public core::interfaces::DeviceRepository {
public:
    MOCK_METHOD(QList<std::shared_ptr<core::entities::Device>>, getAllDevices, (), (override));
    MOCK_METHOD(std::shared_ptr<core::entities::Device>, getDeviceById, (const QString&), (override));
    MOCK_METHOD(bool, addDevice, (std::shared_ptr<core::entities::Device>), (override));
    MOCK_METHOD(bool, removeDevice, (const QString&), (override));
    MOCK_METHOD(bool, updateDevice, (std::shared_ptr<core::entities::Device>), (override));
    MOCK_METHOD(QList<std::shared_ptr<core::entities::Device>>, discoverDevices, (), (override));
    MOCK_METHOD(bool, refreshDeviceStatus, (const QString&), (override));
    MOCK_METHOD(bool, connectToDevice, (const QString&), (override));
    MOCK_METHOD(bool, disconnectFromDevice, (const QString&), (override));
    MOCK_METHOD(bool, isDeviceConnected, (const QString&), (override));
};

class DeviceManagementUseCaseTest : public Test {
protected:
    void SetUp() override {
        mockRepository = std::make_shared<MockDeviceRepository>();
        useCase = std::make_unique<DeviceManagementUseCase>(mockRepository);
    }

    void TearDown() override {
        useCase.reset();
        mockRepository.reset();
    }

    std::shared_ptr<MockDeviceRepository> mockRepository;
    std::unique_ptr<DeviceManagementUseCase> useCase;
};

TEST_F(DeviceManagementUseCaseTest, Constructor_ShouldInitializeCorrectly) {
    EXPECT_NE(useCase, nullptr);
}

TEST_F(DeviceManagementUseCaseTest, GetAllDevices_ShouldCallRepository) {
    QList<std::shared_ptr<core::entities::Device>> expectedDevices;
    expectedDevices.append(std::make_shared<core::entities::Device>("test1", "Test Device 1"));
    expectedDevices.append(std::make_shared<core::entities::Device>("test2", "Test Device 2"));

    EXPECT_CALL(*mockRepository, getAllDevices())
        .WillOnce(Return(expectedDevices));

    auto devices = useCase->getAllDevices();
    EXPECT_EQ(devices.size(), 2);
}

TEST_F(DeviceManagementUseCaseTest, GetDeviceById_ShouldCallRepository) {
    auto expectedDevice = std::make_shared<core::entities::Device>("test1", "Test Device");

    EXPECT_CALL(*mockRepository, getDeviceById("test1"))
        .WillOnce(Return(expectedDevice));

    auto device = useCase->getDeviceById("test1");
    EXPECT_EQ(device, expectedDevice);
}

TEST_F(DeviceManagementUseCaseTest, AddDevice_ShouldCallRepository) {
    auto device = std::make_shared<core::entities::Device>("test1", "Test Device");

    EXPECT_CALL(*mockRepository, addDevice(device))
        .WillOnce(Return(true));

    bool result = useCase->addDevice(device);
    EXPECT_TRUE(result);
}

TEST_F(DeviceManagementUseCaseTest, RemoveDevice_ShouldCallRepository) {
    EXPECT_CALL(*mockRepository, removeDevice("test1"))
        .WillOnce(Return(true));

    bool result = useCase->removeDevice("test1");
    EXPECT_TRUE(result);
}

TEST_F(DeviceManagementUseCaseTest, DiscoverDevices_ShouldCallRepository) {
    QList<std::shared_ptr<core::entities::Device>> expectedDevices;
    expectedDevices.append(std::make_shared<core::entities::Device>("test1", "Test Device"));

    EXPECT_CALL(*mockRepository, discoverDevices())
        .WillOnce(Return(expectedDevices));

    auto devices = useCase->discoverDevices();
    EXPECT_EQ(devices.size(), 1);
}

TEST_F(DeviceManagementUseCaseTest, ConnectToDevice_ShouldCallRepository) {
    EXPECT_CALL(*mockRepository, connectToDevice("test1"))
        .WillOnce(Return(true));

    bool result = useCase->connectToDevice("test1");
    EXPECT_TRUE(result);
}

TEST_F(DeviceManagementUseCaseTest, DisconnectFromDevice_ShouldCallRepository) {
    EXPECT_CALL(*mockRepository, disconnectFromDevice("test1"))
        .WillOnce(Return(true));

    bool result = useCase->disconnectFromDevice("test1");
    EXPECT_TRUE(result);
}

TEST_F(DeviceManagementUseCaseTest, IsDeviceConnected_ShouldCallRepository) {
    EXPECT_CALL(*mockRepository, isDeviceConnected("test1"))
        .WillOnce(Return(true));

    bool result = useCase->isDeviceConnected("test1");
    EXPECT_TRUE(result);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 