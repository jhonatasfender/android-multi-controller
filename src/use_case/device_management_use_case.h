#ifndef DEVICE_MANAGEMENT_USE_CASE_H
#define DEVICE_MANAGEMENT_USE_CASE_H

#include <memory>
#include "../core/interfaces/device_repository.h"
#include "../core/entities/device.h"

namespace use_case
{
    class DeviceManagementUseCase final : public QObject
    {
        Q_OBJECT

    public:
        explicit DeviceManagementUseCase(
            const std::shared_ptr<core::interfaces::DeviceRepository>& repository,
            QObject* parent = nullptr
        );

        QList<std::shared_ptr<core::entities::Device>> discoverAndConnectDevices();
        QList<std::shared_ptr<core::entities::Device>> getConnectedDevices();
        bool refreshDeviceStatus(const QString& deviceId);
        bool connectToDevice(const QString& deviceId);
        bool disconnectFromDevice(const QString& deviceId);

        std::shared_ptr<core::entities::Device> getDeviceById(const QString& deviceId);
        bool updateDeviceName(const QString& deviceId, const QString& name);

        bool connectToAllDevices();
        bool disconnectFromAllDevices();
        bool refreshAllDeviceStatuses();

    signals:
        void deviceDiscovered(std::shared_ptr<core::entities::Device> device);
        void deviceConnected(const QString& deviceId);
        void deviceDisconnected(const QString& deviceId);
        void deviceStatusChanged(const QString& deviceId, core::entities::DeviceStatus status);
        void errorOccurred(const QString& error);

    private:
        std::shared_ptr<core::interfaces::DeviceRepository> m_repository;
    };
}

#endif
