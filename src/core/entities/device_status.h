#ifndef DEVICE_STATUS_H
#define DEVICE_STATUS_H

#include <QObject>

namespace core::entities
{
    Q_NAMESPACE

    enum class DeviceStatus
    {
        Unknown,
        Offline,
        Bootloader,
        Download,
        Recovery,
        Unauthorized,
        Authorized
    };

    Q_ENUM_NS(DeviceStatus)
}

#endif
