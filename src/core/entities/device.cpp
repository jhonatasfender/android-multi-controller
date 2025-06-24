#include "device.h"

#include <utility>

namespace core::entities
{
    Device::Device(QObject* parent) : QObject(parent), m_status(DeviceStatus::Unknown), m_connected(false), m_port(5555)
    {
    }

    Device::Device(
        QString id,
        QString name,
        QObject* parent
    ) : QObject(parent), m_id(std::move(id)), m_name(std::move(name)), m_status(DeviceStatus::Unknown),
        m_connected(false), m_port(5555)
    {
    }

    Device::Device(
        const QString& id,
        const QString& name,
        QObject* parent
    ) : QObject(parent), m_id(id), m_name(name), m_status(DeviceStatus::Unknown), m_connected(false), m_port(5555)
    {
    }

    void Device::setName(const QString& name)
    {
        if (m_name != name)
        {
            m_name = name;
            emit nameChanged();
        }
    }

    void Device::setModel(const QString& model)
    {
        m_model = model;
    }

    void Device::setManufacturer(const QString& manufacturer)
    {
        m_manufacturer = manufacturer;
    }

    void Device::setStatus(const DeviceStatus status)
    {
        if (m_status != status)
        {
            m_status = status;
            emit statusChanged();
        }
    }

    void Device::setConnected(const bool connected)
    {
        if (m_connected != connected)
        {
            m_connected = connected;
            emit connectedChanged();
        }
    }

    void Device::setIpAddress(const QString& ipAddress)
    {
        if (m_ipAddress != ipAddress)
        {
            m_ipAddress = ipAddress;
            emit ipAddressChanged();
        }
    }

    void Device::setPort(const int port)
    {
        if (m_port != port)
        {
            m_port = port;
            emit portChanged();
        }
    }

    bool Device::isOnline() const
    {
        return m_status == DeviceStatus::Authorized;
    }

    bool Device::isAuthorized() const
    {
        return m_status == DeviceStatus::Authorized;
    }

    QString Device::getDisplayName() const
    {
        if (!m_name.isEmpty())
        {
            return m_name;
        }

        if (!m_model.isEmpty())
        {
            return m_model;
        }

        return m_id;
    }
}
