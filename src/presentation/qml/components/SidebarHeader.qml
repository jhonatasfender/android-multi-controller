import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"

Item {
    id: sidebarHeader

    property variant appStyle: null

    property bool isLoading: false

    signal refreshClicked()

    Layout.fillWidth: true
    Layout.preferredHeight: 60

    RowLayout {
        anchors.fill: parent

        Label {
            text: qsTr("Devices")
            font.pixelSize: 20
            font.weight: Font.Medium
            color: appStyle ? appStyle.foreground : "#ffffff"
        }

        Item {
            Layout.fillWidth: true
        }

        ToolButton {
            onClicked: sidebarHeader.refreshClicked()
            enabled: !sidebarHeader.isLoading

            contentItem: ColoredSvg {
                source: "qrc:/icons/icons/refresh.svg"
                width: 20
                height: 20
                sourceSize.width: 20
                sourceSize.height: 20
                svgColor: appStyle ? appStyle.foreground : "#ffffff"
                smooth: true
                mipmap: true
            }

            ToolTip.visible: hovered
            ToolTip.text: qsTr("Refresh devices")
        }
    }
} 