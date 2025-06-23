import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"

RowLayout {
    id: featureCard

    property string iconSource: ""
    property string title: ""
    property string description: ""
    property variant appStyle: null

    spacing: 12

    Rectangle {
        Layout.preferredWidth: 48
        Layout.preferredHeight: 48
        radius: 24
        color: appStyle ? appStyle.accent : "#4CAF50"

        ColoredSvg {
            anchors.centerIn: parent
            source: iconSource
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
            text: title
            font.pixelSize: 16
            font.weight: Font.Medium
            color: appStyle ? appStyle.foreground : "white"
        }

        Label {
            text: description
            font.pixelSize: 14
            color: appStyle ? appStyle.secondaryText : "#9E9E9E"
            wrapMode: Text.WordWrap
        }
    }
} 