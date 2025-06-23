import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import DeviceController 1.0
import "components"

Pane {
    id: deviceCard
    height: 160

    property var device: null
    property variant appStyle: null

    signal connectClicked()

    signal disconnectClicked()

    signal controlClicked()

    background: Rectangle {
        color: appStyle ? appStyle.surface : "#1e1e1e"
        radius: 8
        border.color: device && device.connected ? (appStyle ? appStyle.accent : "#4CAF50") : (appStyle ? appStyle.divider : "#2d2d2d")
        border.width: device && device.connected ? 2 : 1

        StatusIndicator {
            device: deviceCard.device
            appStyle: deviceCard.appStyle
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                radius: 24
                color: appStyle ? appStyle.accent : "#4CAF50"
                opacity: appStyle ? appStyle.accentOpacity : 1.0

                ColoredSvg {
                    anchors.centerIn: parent
                    source: "qrc:/icons/icons/phone.svg"
                    width: 24
                    height: 24
                    sourceSize.width: 24
                    sourceSize.height: 24
                    svgColor: "white"
                    smooth: true
                    mipmap: true
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 4

                Label {
                    text: device ? device.getDisplayName() : "Unknown Device"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: appStyle ? appStyle.foreground : "white"
                    elide: Text.ElideRight
                }

                Label {
                    text: device ? device.model : ""
                    font.pixelSize: 12
                    color: appStyle ? appStyle.secondaryText : "#808080"
                    visible: device && device.model.length > 0
                }

                Label {
                    text: device ? device.manufacturer : ""
                    font.pixelSize: 12
                    color: appStyle ? appStyle.secondaryText : "#808080"
                    visible: device && device.manufacturer.length > 0
                }

                RowLayout {
                    spacing: 8

                    Label {
                        text: device ? device.id : ""
                        font.pixelSize: 10
                        color: appStyle ? appStyle.secondaryText : "#808080"
                        font.family: "Monospace"
                    }

                    Label {
                        text: {
                            if (!device) return ""
                            switch (device.status) {
                                case DeviceStatus.Authorized:
                                    return "Authorized"
                                case DeviceStatus.Unauthorized:
                                    return "Unauthorized"
                                case DeviceStatus.Offline:
                                    return "Offline"
                                case DeviceStatus.Bootloader:
                                    return "Bootloader"
                                case DeviceStatus.Download:
                                    return "Download"
                                case DeviceStatus.Recovery:
                                    return "Recovery"
                                default:
                                    return "Unknown"
                            }
                        }
                        font.pixelSize: 10
                        color: {
                            if (!device) return appStyle ? appStyle.secondaryText : "#808080"
                            switch (device.status) {
                                case DeviceStatus.Authorized:
                                    return appStyle ? appStyle.statusOnline : "#4CAF50"
                                case DeviceStatus.Unauthorized:
                                case DeviceStatus.Offline:
                                    return appStyle ? appStyle.statusOffline : "#808080"
                                case DeviceStatus.Bootloader:
                                case DeviceStatus.Download:
                                case DeviceStatus.Recovery:
                                    return appStyle ? appStyle.statusBusy : "#808080"
                                default:
                                    return appStyle ? appStyle.secondaryText : "#808080"
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: 8

            IconButton {
                Layout.preferredWidth: 120
                text: device && device.connected ? qsTr("Disconnect") : qsTr("Connect")
                onClicked: device && device.connected ? disconnectClicked() : connectClicked()
                enabled: device != null
                iconSource: device && device.connected ? "qrc:/icons/icons/disconnect.svg" : "qrc:/icons/icons/connect.svg"
                backgroundColor: device && device.connected ? (appStyle ? appStyle.buttonDanger : "#d32f2f") : (appStyle ? appStyle.buttonPrimary : "#4CAF50")
                backgroundColorPressed: device && device.connected ? (appStyle ? appStyle.buttonDangerPressed : "#b71c1c") : (appStyle ? appStyle.buttonPrimaryPressed : "#388E3C")
                backgroundColorHovered: device && device.connected ? (appStyle ? appStyle.buttonDangerHovered : "#ff5722") : (appStyle ? appStyle.buttonPrimaryHovered : "#4CAF50")
                appStyle: deviceCard.appStyle
            }

            IconButton {
                Layout.preferredWidth: 120
                text: qsTr("Control")
                enabled: device && device.connected
                visible: device && device.connected
                iconSource: "qrc:/icons/icons/control.svg"
                backgroundColor: appStyle ? appStyle.buttonSuccess : "#4CAF50"
                backgroundColorPressed: appStyle ? appStyle.buttonSuccessPressed : "#388E3C"
                backgroundColorHovered: appStyle ? appStyle.buttonSuccessHovered : "#4CAF50"
                appStyle: deviceCard.appStyle
                onClicked: controlClicked()
            }
        }
    }

    states: [
        State {
            name: "hovered"
            when: mouseArea.containsMouse
            PropertyChanges {
                target: deviceCard
                scale: 1.02
            }
        }
    ]

    transitions: [
        Transition {
            to: "hovered"
            reversible: true
            NumberAnimation {
                properties: "scale"
                duration: 150
                easing.type: Easing.OutQuad
            }
        }
    ]

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }
}