#include "adb_device_repository.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>

namespace infrastructure::persistence
{
    AdbDeviceRepository::AdbDeviceRepository(
        const std::shared_ptr<adb::AdbService>& adbService,
        QObject* parent
    ) : QObject(parent), m_adbService(adbService)
    {
        if (m_adbService)
        {
            connect(
                m_adbService.get(),
                &adb::AdbService::devicesUpdated,
                this,
                &AdbDeviceRepository::onDevicesUpdated
            );
            connect(
                m_adbService.get(),
                &adb::AdbService::deviceConnected,
                this,
                &AdbDeviceRepository::onDeviceConnected
            );
            connect(
                m_adbService.get(),
                &adb::AdbService::deviceDisconnected,
                this,
                &AdbDeviceRepository::onDeviceDisconnected
            );
            connect(
                m_adbService.get(),
                &adb::AdbService::errorOccurred,
                this,
                &AdbDeviceRepository::onErrorOccurred
            );
        }
    }

    AdbDeviceRepository::~AdbDeviceRepository() = default;

    QList<std::shared_ptr<core::entities::Device>> AdbDeviceRepository::getAllDevices()
    {
        return m_devices;
    }

    std::shared_ptr<core::entities::Device> AdbDeviceRepository::getDeviceById(const QString& id)
    {
        for (const auto& device : m_devices)
        {
            if (device->id() == id)
            {
                return device;
            }
        }
        return nullptr;
    }

    bool AdbDeviceRepository::addDevice(const std::shared_ptr<core::entities::Device> device)
    {
        if (!device)
        {
            return false;
        }

        for (const auto& existingDevice : m_devices)
        {
            if (existingDevice->id() == device->id())
            {
                return false;
            }
        }

        m_devices.append(device);
        return true;
    }

    bool AdbDeviceRepository::removeDevice(const QString& id)
    {
        for (int i = 0; i < m_devices.size(); ++i)
        {
            if (m_devices[i]->id() == id)
            {
                m_devices.removeAt(i);
                return true;
            }
        }
        return false;
    }

    bool AdbDeviceRepository::updateDevice(const std::shared_ptr<core::entities::Device> device)
    {
        if (!device)
        {
            return false;
        }

        for (int i = 0; i < m_devices.size(); ++i)
        {
            if (m_devices[i]->id() == device->id())
            {
                m_devices[i] = device;
                return true;
            }
        }
        return false;
    }

    QList<std::shared_ptr<core::entities::Device>> AdbDeviceRepository::discoverDevices()
    {
        if (m_adbService)
        {
            m_devices = m_adbService->getConnectedDevices();
        }
        return m_devices;
    }

    bool AdbDeviceRepository::refreshDeviceStatus(const QString& id)
    {
        if (!m_adbService)
        {
            return false;
        }

        const bool isConnected = m_adbService->isDeviceConnected(id);

        if (const auto device = getDeviceById(id))
        {
            device->setStatus(
                isConnected
                    ? core::entities::DeviceStatus::Authorized
                    : core::entities::DeviceStatus::Offline
            );
            device->setConnected(isConnected);
            return true;
        }

        return false;
    }

    bool AdbDeviceRepository::connectToDevice(const QString& id)
    {
        if (isDeviceConnected(id))
        {
            return true;
        }

        if (m_adbService && m_adbService->connectToDevice(id))
        {
            return refreshDeviceStatus(id);
        }
        return false;
    }

    bool AdbDeviceRepository::disconnectFromDevice(const QString& id)
    {
        if (m_adbService && m_adbService->disconnectFromDevice(id))
        {
            if (const auto device = getDeviceById(id))
            {
                device->setStatus(core::entities::DeviceStatus::Offline);
                device->setConnected(false);
                return true;
            }
        }
        return false;
    }

    bool AdbDeviceRepository::isDeviceConnected(const QString& id)
    {
        if (m_adbService)
        {
            return m_adbService->isDeviceConnected(id);
        }
        return false;
    }

    void AdbDeviceRepository::onDevicesUpdated(const QList<std::shared_ptr<core::entities::Device>>& devices)
    {
        m_devices = devices;
    }

    void AdbDeviceRepository::onDeviceConnected(const std::shared_ptr<core::entities::Device>& device)
    {
        if (device)
        {
            addDevice(device);
        }
    }

    void AdbDeviceRepository::onDeviceDisconnected(const QString& deviceId)
    {
        if (const auto device = getDeviceById(deviceId))
        {
            device->setStatus(core::entities::DeviceStatus::Offline);
            device->setConnected(false);
        }
    }

    void AdbDeviceRepository::onErrorOccurred(const QString& error)
    {
        qWarning() << "AdbDeviceRepository error:" << error;
    }
}
