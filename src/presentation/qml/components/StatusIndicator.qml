import QtQuick 2.15
import DeviceController 1.0
import "../"

Rectangle {
    id: statusIndicator

    property var device: null
    property variant appStyle: null

    width: 8
    height: 8
    radius: 4

    color: {
        if (!device) return appStyle ? appStyle.secondaryText : "#808080"
        switch (device.status) {
            case DeviceStatus.Authorized:
                return appStyle ? appStyle.statusOnline : "#4CAF50"
            case DeviceStatus.Unauthorized:
            case DeviceStatus.Offline:
                return appStyle ? appStyle.statusOffline : "#f44336"
            case DeviceStatus.Bootloader:
            case DeviceStatus.Download:
            case DeviceStatus.Recovery:
                return appStyle ? appStyle.statusBusy : "#ff9800"
            default:
                return appStyle ? appStyle.secondaryText : "#808080"
        }
    }
} 