import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"

Button {
    id: iconButton

    property string iconSource: ""
    property color iconColor: "white"
    property color backgroundColor: appStyle ? appStyle.buttonPrimary : "#2196f3"
    property color backgroundColorPressed: appStyle ? appStyle.buttonPrimaryPressed : "#1976d2"
    property color backgroundColorHovered: appStyle ? appStyle.buttonPrimaryHovered : "#42a5f5"
    property variant appStyle: null

    background: Rectangle {
        color: parent.pressed ? iconButton.backgroundColorPressed :
            parent.hovered ? iconButton.backgroundColorHovered :
                iconButton.backgroundColor
        radius: 4
    }

    contentItem: RowLayout {
        spacing: 4

        ColoredSvg {
            source: iconButton.iconSource
            width: 16
            height: 16
            sourceSize.width: 16
            sourceSize.height: 16
            svgColor: iconButton.iconColor
            smooth: true
            mipmap: true
        }

        Label {
            text: parent.parent.text
            color: iconButton.iconColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
} 