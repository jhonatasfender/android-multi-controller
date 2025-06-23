#include "device_management_use_case.h"

namespace use_case
{
    DeviceManagementUseCase::DeviceManagementUseCase(
        const std::shared_ptr<core::interfaces::DeviceRepository>& repository,
        QObject* parent
    ) : QObject(parent), m_repository(repository)
    {
    }

    QList<std::shared_ptr<core::entities::Device>> DeviceManagementUseCase::discoverAndConnectDevices()
    {
        try
        {
            auto devices = m_repository->discoverDevices();

            for (const auto& device : devices)
            {
                emit deviceDiscovered(device);

                if (m_repository->connectToDevice(device->id()))
                {
                    emit deviceConnected(device->id());
                }
            }

            return devices;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error discovering devices: %1").arg(e.what()));
            return QList<std::shared_ptr<core::entities::Device>>();
        }
    }

    QList<std::shared_ptr<core::entities::Device>> DeviceManagementUseCase::getConnectedDevices()
    {
        try
        {
            auto allDevices = m_repository->getAllDevices();
            QList<std::shared_ptr<core::entities::Device>> connectedDevices;

            for (auto& device : allDevices)
            {
                if (device->connected())
                {
                    connectedDevices.append(device);
                }
            }

            return connectedDevices;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error getting connected devices: %1").arg(e.what()));
            return QList<std::shared_ptr<core::entities::Device>>();
        }
    }

    bool DeviceManagementUseCase::refreshDeviceStatus(const QString& deviceId)
    {
        try
        {
            if (m_repository->refreshDeviceStatus(deviceId))
            {
                if (const auto device = m_repository->getDeviceById(deviceId))
                {
                    emit deviceStatusChanged(deviceId, device->status());
                }
                return true;
            }
            return false;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error refreshing device status: %1").arg(e.what()));
            return false;
        }
    }

    bool DeviceManagementUseCase::connectToDevice(const QString& deviceId)
    {
        try
        {
            if (m_repository->connectToDevice(deviceId))
            {
                emit deviceConnected(deviceId);
                return true;
            }
            return false;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error connecting to a device: %1").arg(e.what()));
            return false;
        }
    }

    bool DeviceManagementUseCase::disconnectFromDevice(const QString& deviceId)
    {
        try
        {
            if (m_repository->disconnectFromDevice(deviceId))
            {
                emit deviceDisconnected(deviceId);
                return true;
            }
            return false;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error disconnecting from a device: %1").arg(e.what()));
            return false;
        }
    }

    std::shared_ptr<core::entities::Device> DeviceManagementUseCase::getDeviceById(const QString& deviceId)
    {
        try
        {
            return m_repository->getDeviceById(deviceId);
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error getting device: %1").arg(e.what()));
            return nullptr;
        }
    }

    bool DeviceManagementUseCase::updateDeviceName(const QString& deviceId, const QString& name)
    {
        try
        {
            if (const auto device = m_repository->getDeviceById(deviceId))
            {
                device->setName(name);
                return m_repository->updateDevice(device);
            }
            return false;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error updating device name: %1").arg(e.what()));
            return false;
        }
    }

    bool DeviceManagementUseCase::connectToAllDevices()
    {
        try
        {
            auto devices = m_repository->getAllDevices();
            bool allConnected = true;

            for (const auto& device : devices)
            {
                if (!connectToDevice(device->id()))
                {
                    allConnected = false;
                }
            }

            return allConnected;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error connecting to all devices: %1").arg(e.what()));
            return false;
        }
    }

    bool DeviceManagementUseCase::disconnectFromAllDevices()
    {
        try
        {
            auto devices = m_repository->getAllDevices();
            bool allDisconnected = true;

            for (const auto& device : devices)
            {
                if (!disconnectFromDevice(device->id()))
                {
                    allDisconnected = false;
                }
            }

            return allDisconnected;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error disconnecting from all devices: %1").arg(e.what()));
            return false;
        }
    }

    bool DeviceManagementUseCase::refreshAllDeviceStatuses()
    {
        try
        {
            auto devices = m_repository->getAllDevices();
            bool allRefreshed = true;

            for (const auto& device : devices)
            {
                if (!m_repository->refreshDeviceStatus(device->id()))
                {
                    allRefreshed = false;
                }
            }

            return allRefreshed;
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(QString("Error is refreshing all device statuses: %1").arg(e.what()));
            return false;
        }
    }
}
