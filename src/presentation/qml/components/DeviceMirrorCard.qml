import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import DeviceController 1.0

Rectangle {
    id: deviceMirrorCard

    property var device: null
    property variant appStyle: null
    property bool isMirroring: false
    property var capturedImage: null

    signal clicked()

    color: appStyle ? appStyle.surface : "#1e1e1e"
    radius: 8
    border.color: device && device.connected ? (appStyle ? appStyle.accent : "#4CAF50") : (appStyle ? appStyle.divider : "#2d2d2d")
    border.width: device && device.connected ? 2 : 1

    states: [
        State {
            name: "hovered"
            when: mouseArea.containsMouse
            PropertyChanges {
                target: deviceMirrorCard
                scale: 1.02
            }
        }
    ]

    transitions: [
        Transition {
            from: ""
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
        onClicked: deviceMirrorCard.clicked()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 12
                color: device && device.connected ? (appStyle ? appStyle.accent : "#4CAF50") : (appStyle ? appStyle.secondaryText : "#808080")

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/icons/icons/phone.svg"
                    width: 12
                    height: 12
                    sourceSize.width: 12
                    sourceSize.height: 12
                    smooth: true
                    mipmap: true
                }
            }

            Label {
                text: device ? device.getDisplayName() : "Unknown Device"
                font.pixelSize: 12
                font.weight: Font.Medium
                color: appStyle ? appStyle.foreground : "white"
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Rectangle {
                Layout.preferredWidth: 8
                Layout.preferredHeight: 8
                radius: 4
                color: {
                    if (!device) return appStyle ? appStyle.secondaryText : "#808080"
                    if (!device.connected) return appStyle ? appStyle.error : "#f44336"
                    if (isMirroring) return appStyle ? appStyle.success : "#4CAF50"
                    return appStyle ? appStyle.accent : "#4CAF50"
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: appStyle ? appStyle.background : "#121212"
            radius: 4
            border.color: appStyle ? appStyle.divider : "#2d2d2d"
            border.width: 1

            Image {
                id: screenImage
                anchors.fill: parent
                anchors.margins: 4
                source: capturedImage
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
                visible: capturedImage && isMirroring
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                color: appStyle ? appStyle.background : "#121212"
                radius: 2
                border.color: appStyle ? appStyle.divider : "#2d2d2d"
                border.width: 1
                visible: !capturedImage || !isMirroring

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        source: "qrc:/icons/icons/phone.svg"
                        width: 32
                        height: 32
                        sourceSize.width: 32
                        sourceSize.height: 32
                        smooth: true
                        mipmap: true
                    }

                    Label {
                        text: {
                            if (!device) return qsTr("No Device")
                            if (!device.connected) return qsTr("Disconnected")
                            if (!isMirroring) return qsTr("Not Mirroring")
                            return qsTr("Loading...")
                        }
                        font.pixelSize: 10
                        color: appStyle ? appStyle.secondaryText : "#808080"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 20
                color: Qt.rgba(0, 0, 0, 0.7)
                visible: device && device.connected

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 4

                    Label {
                        text: device ? device.model : ""
                        font.pixelSize: 8
                        color: "white"
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Label {
                        text: isMirroring ? qsTr("LIVE") : qsTr("OFF")
                        font.pixelSize: 8
                        font.weight: Font.Bold
                        color: isMirroring ? "#4CAF50" : "#FF5722"
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Button {
                Layout.preferredWidth: 60
                Layout.preferredHeight: 24
                text: isMirroring ? qsTr("Stop") : qsTr("Start")
                font.pixelSize: 8
                background: Rectangle {
                    color: isMirroring ? (appStyle ? appStyle.buttonDanger : "#f44336") : (appStyle ? appStyle.buttonPrimary : "#2196f3")
                    radius: 4
                }
                enabled: device && device.connected
                onClicked: deviceMirrorCard.clicked()
            }

            Button {
                Layout.preferredWidth: 60
                Layout.preferredHeight: 24
                text: qsTr("Control")
                font.pixelSize: 8
                background: Rectangle {
                    color: appStyle ? appStyle.buttonSecondary : "#666666"
                    radius: 4
                }
                enabled: device && device.connected
                onClicked: {
                    if (device) {
                        mainStack.push("qrc:/qml/DeviceControlView.qml", {
                            "device": device
                        })
                    }
                }
            }
        }
    }
} 