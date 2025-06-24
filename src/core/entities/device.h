#ifndef DEVICE_H
#define DEVICE_H

#include "device_status.h"

namespace core::entities
{
    class Device final : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString id READ id CONSTANT)
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
        Q_PROPERTY(QString model READ model CONSTANT)
        Q_PROPERTY(QString manufacturer READ manufacturer CONSTANT)
        Q_PROPERTY(DeviceStatus status READ status WRITE setStatus NOTIFY statusChanged)
        Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
        Q_PROPERTY(QString ipAddress READ ipAddress WRITE setIpAddress NOTIFY ipAddressChanged)
        Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

    public:
        explicit Device(QObject* parent = nullptr);
        Device(QString id, QString name, QObject* parent);
        Device(const QString& id, const QString& name, QObject* parent = nullptr);

        QString id() const { return m_id; }
        QString name() const { return m_name; }
        QString model() const { return m_model; }
        QString manufacturer() const { return m_manufacturer; }
        DeviceStatus status() const { return m_status; }
        bool connected() const { return m_connected; }
        QString ipAddress() const { return m_ipAddress; }
        int port() const { return m_port; }

        void setName(const QString& name);
        void setModel(const QString& model);
        void setManufacturer(const QString& manufacturer);
        void setStatus(DeviceStatus status);
        void setConnected(bool connected);
        void setIpAddress(const QString& ipAddress);
        void setPort(int port);

        bool isOnline() const;
        bool isAuthorized() const;
        Q_INVOKABLE QString getDisplayName() const;

    signals:
        void nameChanged();
        void statusChanged();
        void connectedChanged();
        void ipAddressChanged();
        void portChanged();

    private:
        QString m_id;
        QString m_name;
        QString m_model;
        QString m_manufacturer;
        DeviceStatus m_status;
        bool m_connected;
        QString m_ipAddress;
        int m_port;
    };
}

#endif
