#include "device_repository_impl.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace infrastructure::persistence
{
    DeviceRepositoryImpl::DeviceRepositoryImpl(QObject* parent) : QObject(parent)
    {
        loadDevices();
    }

    DeviceRepositoryImpl::~DeviceRepositoryImpl() = default;

    QList<std::shared_ptr<core::entities::Device>> DeviceRepositoryImpl::getAllDevices()
    {
        return m_devices;
    }

    std::shared_ptr<core::entities::Device> DeviceRepositoryImpl::getDeviceById(const QString& id)
    {
        for (auto& device : m_devices)
        {
            if (device->id() == id)
            {
                return device;
            }
        }
        return nullptr;
    }

    bool DeviceRepositoryImpl::addDevice(const std::shared_ptr<core::entities::Device> device)
    {
        for (const auto& existingDevice : m_devices)
        {
            if (existingDevice->id() == device->id())
            {
                return false;
            }
        }

        m_devices.append(device);
        saveDevices();
        return true;
    }

    bool DeviceRepositoryImpl::removeDevice(const QString& id)
    {
        for (int i = 0; i < m_devices.size(); ++i)
        {
            if (m_devices[i]->id() == id)
            {
                m_devices.removeAt(i);
                saveDevices();
                return true;
            }
        }
        return false;
    }

    bool DeviceRepositoryImpl::updateDevice(const std::shared_ptr<core::entities::Device> device)
    {
        for (int i = 0; i < m_devices.size(); ++i)
        {
            if (m_devices[i]->id() == device->id())
            {
                m_devices[i] = device;
                saveDevices();
                return true;
            }
        }
        return false;
    }

    QList<std::shared_ptr<core::entities::Device>> DeviceRepositoryImpl::discoverDevices()
    {
        return m_devices;
    }

    bool DeviceRepositoryImpl::refreshDeviceStatus(const QString& id)
    {
        if (auto device = getDeviceById(id))
        {
            return true;
        }
        return false;
    }

    bool DeviceRepositoryImpl::connectToDevice(const QString& id)
    {
        if (const auto device = getDeviceById(id))
        {
            device->setConnected(true);
            device->setStatus(core::entities::DeviceStatus::Authorized);
            saveDevices();
            return true;
        }
        return false;
    }

    bool DeviceRepositoryImpl::disconnectFromDevice(const QString& id)
    {
        if (const auto device = getDeviceById(id))
        {
            device->setConnected(false);
            device->setStatus(core::entities::DeviceStatus::Offline);
            saveDevices();
            return true;
        }
        return false;
    }

    bool DeviceRepositoryImpl::isDeviceConnected(const QString& id)
    {
        const auto device = getDeviceById(id);
        return device ? device->connected() : false;
    }

    void DeviceRepositoryImpl::loadDevices()
    {
        QFile file(QDir::homePath() + "/.android_controller/devices.json");
        if (!file.exists())
        {
            return;
        }

        if (file.open(QIODevice::ReadOnly))
        {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

            for (QJsonArray devicesArray = doc.array(); const QJsonValue& value : devicesArray)
            {
                QJsonObject deviceObj = value.toObject();

                auto device = std::make_shared<core::entities::Device>(
                    deviceObj["id"].toString(),
                    deviceObj["name"].toString()
                );

                device->setModel(deviceObj["model"].toString());
                device->setManufacturer(deviceObj["manufacturer"].toString());
                device->setStatus(static_cast<core::entities::DeviceStatus>(
                        deviceObj["status"].toInt())
                );
                device->setConnected(deviceObj["connected"].toBool());
                device->setIpAddress(deviceObj["ipAddress"].toString());
                device->setPort(deviceObj["port"].toInt());

                m_devices.append(device);
            }

            file.close();
        }
    }

    void DeviceRepositoryImpl::saveDevices()
    {
        const QDir dir(QDir::homePath() + "/.android_controller");

        if (QFile file(dir.path() + "/devices.json"); file.open(QIODevice::WriteOnly))
        {
            QJsonArray devicesArray;

            for (const auto& device : m_devices)
            {
                QJsonObject deviceObj;
                deviceObj["id"] = device->id();
                deviceObj["name"] = device->name();
                deviceObj["model"] = device->model();
                deviceObj["manufacturer"] = device->manufacturer();
                deviceObj["status"] = static_cast<int>(device->status());
                deviceObj["connected"] = device->connected();
                deviceObj["ipAddress"] = device->ipAddress();
                deviceObj["port"] = device->port();

                devicesArray.append(deviceObj);
            }

            const QJsonDocument doc(devicesArray);
            file.write(doc.toJson());
            file.close();
        }
    }
}
