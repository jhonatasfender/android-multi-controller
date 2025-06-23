import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import "components"

Item {
    id: welcomeView

    property variant appStyle: null

    Rectangle {
        anchors.fill: parent
        color: appStyle ? appStyle.background : "#121212"
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 32
        width: Math.min(parent.width * 0.8, 600)

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            Rectangle {
                Layout.preferredWidth: 120
                Layout.preferredHeight: 120
                radius: 60
                color: appStyle ? appStyle.accent : "#4CAF50"
                opacity: appStyle ? appStyle.accentOpacity : 1.0

                ColoredSvg {
                    anchors.centerIn: parent
                    source: "qrc:/icons/icons/phone.svg"
                    width: 48
                    height: 48
                    sourceSize.width: 48
                    sourceSize.height: 48
                    svgColor: "white"
                    smooth: true
                    mipmap: true
                }
            }

            Label {
                text: qsTr("Multi-Device Android Controller")
                font.pixelSize: 28
                font.weight: Font.Light
                color: appStyle ? appStyle.foreground : "white"
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                text: qsTr("Control multiple Android devices simultaneously")
                font.pixelSize: 16
                color: appStyle ? appStyle.secondaryText : "#9E9E9E"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
        }

        Pane {
            Layout.fillWidth: true

            background: Rectangle {
                color: appStyle ? appStyle.surface : "#333333"
                radius: 8
                border.color: appStyle ? appStyle.divider : "#444444"
                border.width: 1
            }

            contentItem: ColumnLayout {
                id: featuresColumn
                anchors.margins: 24
                spacing: 16

                Label {
                    text: qsTr("Features")
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: appStyle ? appStyle.foreground : "white"
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    rowSpacing: 12
                    columnSpacing: 16

                    FeatureCard {
                        iconSource: "qrc:/icons/icons/search.svg"
                        title: qsTr("Device Discovery")
                        description: qsTr("Automatically detect connected Android devices")
                        appStyle: welcomeView.appStyle
                    }

                    FeatureCard {
                        iconSource: "qrc:/icons/icons/gamepad.svg"
                        title: qsTr("Remote Control")
                        description: qsTr("Control devices remotely via ADB")
                        appStyle: welcomeView.appStyle
                    }

                    FeatureCard {
                        iconSource: "qrc:/icons/icons/phone.svg"
                        title: qsTr("Multi-Device")
                        description: qsTr("Manage multiple devices simultaneously")
                        appStyle: welcomeView.appStyle
                    }

                    FeatureCard {
                        iconSource: "qrc:/icons/icons/robot.svg"
                        title: qsTr("Automation")
                        description: qsTr("Create and run automation scripts")
                        appStyle: welcomeView.appStyle
                    }

                    FeatureCard {
                        iconSource: "qrc:/icons/icons/chart.svg"
                        title: qsTr("Real-time Monitoring")
                        description: qsTr("Monitor device status and performance")
                        appStyle: welcomeView.appStyle
                    }

                    FeatureCard {
                        iconSource: "qrc:/icons/icons/lightning.svg"
                        title: qsTr("Fast & Responsive")
                        description: qsTr("Modern UI with smooth interactions")
                        appStyle: welcomeView.appStyle
                    }
                }
            }
        }

        Pane {
            Layout.fillWidth: true

            background: Rectangle {
                color: appStyle ? appStyle.surface : "#333333"
                radius: 8
                border.color: appStyle ? appStyle.divider : "#444444"
                border.width: 1
            }

            contentItem: ColumnLayout {
                id: gettingStartedColumn
                anchors.margins: 24
                spacing: 16

                Label {
                    text: qsTr("Getting Started")
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: appStyle ? appStyle.foreground : "white"
                }

                ColumnLayout {
                    spacing: 8

                    RowLayout {
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            radius: 12
                            color: appStyle ? appStyle.accent : "#4CAF50"

                            Label {
                                anchors.centerIn: parent
                                text: "1"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: "white"
                            }
                        }

                        Label {
                            text: qsTr("Enable USB debugging on your Android devices")
                            font.pixelSize: 14
                            color: appStyle ? appStyle.foreground : "white"
                            wrapMode: Text.WordWrap
                        }
                    }

                    RowLayout {
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            radius: 12
                            color: appStyle ? appStyle.accent : "#4CAF50"

                            Label {
                                anchors.centerIn: parent
                                text: "2"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: "white"
                            }
                        }

                        Label {
                            text: qsTr("Connect devices via USB or enable wireless debugging")
                            font.pixelSize: 14
                            color: appStyle ? appStyle.foreground : "white"
                            wrapMode: Text.WordWrap
                        }
                    }

                    RowLayout {
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            radius: 12
                            color: appStyle ? appStyle.accent : "#4CAF50"

                            Label {
                                anchors.centerIn: parent
                                text: "3"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: "white"
                            }
                        }

                        Label {
                            text: qsTr("Click 'Refresh Devices' to discover connected devices")
                            font.pixelSize: 14
                            color: appStyle ? appStyle.foreground : "white"
                            wrapMode: Text.WordWrap
                        }
                    }

                    RowLayout {
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            radius: 12
                            color: appStyle ? appStyle.accent : "#4CAF50"

                            Label {
                                anchors.centerIn: parent
                                text: "4"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: "white"
                            }
                        }

                        Label {
                            text: qsTr("Click 'Connect' on each device to establish connection")
                            font.pixelSize: 14
                            color: appStyle ? appStyle.foreground : "white"
                            wrapMode: Text.WordWrap
                        }
                    }

                    RowLayout {
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            radius: 12
                            color: appStyle ? appStyle.accent : "#4CAF50"

                            Label {
                                anchors.centerIn: parent
                                text: "5"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: "white"
                            }
                        }

                        Label {
                            text: qsTr("Use the control panel to manage your devices")
                            font.pixelSize: 14
                            color: appStyle ? appStyle.foreground : "white"
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            IconButton {
                text: qsTr("Refresh Devices")
                onClicked: deviceListViewModel.refreshDevices()
                enabled: !deviceListViewModel.isLoading
                iconSource: "qrc:/icons/icons/refresh.svg"
                appStyle: welcomeView.appStyle
            }

            IconButton {
                text: qsTr("Documentation")
                flat: true
                iconSource: "qrc:/icons/icons/documentation.svg"
                iconColor: appStyle ? appStyle.accent : "#4CAF50"
                backgroundColor: "transparent"
                backgroundColorPressed: appStyle ? appStyle.hintText : "#666666"
                backgroundColorHovered: appStyle ? appStyle.hintText : "#666666"
                appStyle: welcomeView.appStyle
            }
        }
    }
}