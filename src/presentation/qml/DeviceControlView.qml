import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import "components"

Item {
    id: deviceControlView

    property var device: null
    property variant appStyle: Style

    Rectangle {
        anchors.fill: parent
        color: appStyle.background
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        Pane {
            Layout.fillWidth: true
            Layout.preferredHeight: 80

            background: Rectangle {
                color: appStyle.surface
                radius: 8
                border.color: appStyle.divider
                border.width: 1
            }

            contentItem: RowLayout {
                spacing: 16

                Rectangle {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48
                    radius: 24
                    color: appStyle.accent
                    opacity: appStyle.accentOpacity

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
                    spacing: 4

                    Label {
                        text: device ? device.getDisplayName() : "Unknown Device"
                        font.pixelSize: 18
                        font.weight: Font.Medium
                        color: appStyle.foreground
                    }

                    Label {
                        text: device ? (device.model + " • " + device.manufacturer) : ""
                        font.pixelSize: 14
                        color: appStyle.secondaryText
                        visible: device && device.model.length > 0
                    }
                }

                IconButton {
                    text: qsTr("Back")
                    iconSource: "qrc:/icons/icons/close.svg"
                    backgroundColor: appStyle.buttonSecondary
                    backgroundColorPressed: appStyle.buttonSecondaryPressed
                    backgroundColorHovered: appStyle.buttonSecondaryHovered
                    appStyle: deviceControlView.appStyle
                    onClicked: {
                        if (mainStack.depth > 1) {
                            mainStack.pop()
                        }
                    }
                }
            }
        }

        Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true

            background: Rectangle {
                color: appStyle.surface
                radius: 8
                border.color: appStyle.divider
                border.width: 1
            }

            contentItem: ColumnLayout {
                anchors.fill: parent
                spacing: 16

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: appStyle.background
                    radius: 4
                    border.color: appStyle.divider
                    border.width: 1
                    border.style: Qt.DashedLine

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 16

                        ColoredSvg {
                            Layout.alignment: Qt.AlignHCenter
                            source: "qrc:/icons/icons/control.svg"
                            width: 64
                            height: 64
                            sourceSize.width: 64
                            sourceSize.height: 64
                            svgColor: appStyle.secondaryText
                            smooth: true
                            mipmap: true
                        }

                        Label {
                            text: qsTr("Device Screen Mirroring")
                            font.pixelSize: 18
                            font.weight: Font.Medium
                            color: appStyle.foreground
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Label {
                            text: qsTr("The device screen will be displayed here")
                            font.pixelSize: 14
                            color: appStyle.secondaryText
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                        }

                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter
                            spacing: 12

                            IconButton {
                                text: qsTr("Start Mirroring")
                                iconSource: "qrc:/icons/icons/fullscreen.svg"
                                backgroundColor: appStyle.buttonPrimary
                                backgroundColorPressed: appStyle.buttonPrimaryPressed
                                backgroundColorHovered: appStyle.buttonPrimaryHovered
                                appStyle: deviceControlView.appStyle
                            }

                            IconButton {
                                text: qsTr("Take Screenshot")
                                iconSource: "qrc:/icons/icons/control.svg"
                                backgroundColor: appStyle.buttonSuccess
                                backgroundColorPressed: appStyle.buttonSuccessPressed
                                backgroundColorHovered: appStyle.buttonSuccessHovered
                                appStyle: deviceControlView.appStyle
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    spacing: 12

                    IconButton {
                        Layout.preferredWidth: 120
                        text: qsTr("Home")
                        iconSource: "qrc:/icons/icons/control.svg"
                        backgroundColor: appStyle.buttonSecondary
                        backgroundColorPressed: appStyle.buttonSecondaryPressed
                        backgroundColorHovered: appStyle.buttonSecondaryHovered
                        appStyle: deviceControlView.appStyle
                    }

                    IconButton {
                        Layout.preferredWidth: 120
                        text: qsTr("Back")
                        iconSource: "qrc:/icons/icons/control.svg"
                        backgroundColor: appStyle.buttonSecondary
                        backgroundColorPressed: appStyle.buttonSecondaryPressed
                        backgroundColorHovered: appStyle.buttonSecondaryHovered
                        appStyle: deviceControlView.appStyle
                    }

                    IconButton {
                        Layout.preferredWidth: 120
                        text: qsTr("Recent")
                        iconSource: "qrc:/icons/icons/control.svg"
                        backgroundColor: appStyle.buttonSecondary
                        backgroundColorPressed: appStyle.buttonSecondaryPressed
                        backgroundColorHovered: appStyle.buttonSecondaryHovered
                        appStyle: deviceControlView.appStyle
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    IconButton {
                        Layout.preferredWidth: 120
                        text: qsTr("Settings")
                        iconSource: "qrc:/icons/icons/control.svg"
                        backgroundColor: appStyle.buttonSecondary
                        backgroundColorPressed: appStyle.buttonSecondaryPressed
                        backgroundColorHovered: appStyle.buttonSecondaryHovered
                        appStyle: deviceControlView.appStyle
                    }
                }
            }
        }
    }
}