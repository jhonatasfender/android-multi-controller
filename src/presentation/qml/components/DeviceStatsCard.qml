import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"

Pane {
    id: deviceStatsCard

    property variant appStyle: null

    property int connectedDeviceCount: 0

    Layout.fillWidth: true
    Layout.preferredHeight: 80

    background: Rectangle {
        color: appStyle ? appStyle.surface : "#1e1e1e"
        radius: 8
        border.color: appStyle ? appStyle.divider : "#2d2d2d"
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16

        Label {
            text: qsTr("Connected Devices")
            font.pixelSize: 14
            color: appStyle ? appStyle.secondaryText : "#808080"
        }

        Label {
            text: deviceStatsCard.connectedDeviceCount
            font.pixelSize: 24
            font.weight: Font.Medium
            color: appStyle ? appStyle.foreground : "#ffffff"
        }
    }
} 