import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"

Pane {
    id: sidebar

    property variant appStyle: null

    property bool isLoading: false
    property int connectedDeviceCount: 0
    property var deviceList: []

    signal refreshClicked()

    signal connectAllClicked()

    signal disconnectAllClicked()

    signal connectDeviceClicked(string deviceId)

    signal disconnectDeviceClicked(string deviceId)

    signal controlDeviceClicked(var device)

    signal multiDeviceMirrorClicked()

    Layout.preferredWidth: Math.min(350, parent.width * 0.3)
    Layout.fillHeight: true
    Layout.maximumWidth: 800
    Layout.minimumWidth: 450

    background: Rectangle {
        color: appStyle ? appStyle.surface : "#1e1e1e"
        border.color: appStyle ? appStyle.divider : "#2d2d2d"
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        SidebarHeader {
            appStyle: sidebar.appStyle
            isLoading: sidebar.isLoading
            onRefreshClicked: sidebar.refreshClicked()
        }

        DeviceStatsCard {
            appStyle: sidebar.appStyle
            connectedDeviceCount: sidebar.connectedDeviceCount
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: deviceListView
                model: sidebar.deviceList
                spacing: 8

                delegate: deviceCardComponent
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            IconButton {
                Layout.fillWidth: true
                text: qsTr("Multi-Device Mirror")
                onClicked: sidebar.multiDeviceMirrorClicked()
                enabled: sidebar.connectedDeviceCount > 0
                iconSource: "qrc:/icons/icons/fullscreen.svg"
                backgroundColor: appStyle ? appStyle.buttonSuccess : "#4CAF50"
                backgroundColorPressed: appStyle ? appStyle.buttonSuccessPressed : "#45a049"
                backgroundColorHovered: appStyle ? appStyle.buttonSuccessHovered : "#45a049"
                appStyle: sidebar.appStyle
            }

            IconButton {
                Layout.fillWidth: true
                text: qsTr("Connect All")
                onClicked: sidebar.connectAllClicked()
                enabled: sidebar.connectedDeviceCount < (sidebar.deviceList ? sidebar.deviceList.count : 0)
                iconSource: "qrc:/icons/icons/connect.svg"
                appStyle: sidebar.appStyle
            }

            IconButton {
                Layout.fillWidth: true
                text: qsTr("Disconnect All")
                onClicked: sidebar.disconnectAllClicked()
                enabled: sidebar.connectedDeviceCount > 0
                iconSource: "qrc:/icons/icons/disconnect.svg"
                backgroundColor: appStyle ? appStyle.buttonDanger : "#f44336"
                backgroundColorPressed: appStyle ? appStyle.buttonDangerPressed : "#d32f2f"
                backgroundColorHovered: appStyle ? appStyle.buttonDangerHovered : "#d32f2f"
                appStyle: sidebar.appStyle
            }
        }
    }

    Component {
        id: deviceCardComponent

        Loader {
            width: deviceListView.width
            source: "qrc:/qml/DeviceCard.qml"

            onLoaded: {
                item.device = modelData
                item.appStyle = sidebar.appStyle
                item.connectClicked.connect(function () {
                    sidebar.connectDeviceClicked(modelData.id)
                })
                item.disconnectClicked.connect(function () {
                    sidebar.disconnectDeviceClicked(modelData.id)
                })
                item.controlClicked.connect(function () {
                    sidebar.controlDeviceClicked(modelData)
                })
            }
        }
    }
} 