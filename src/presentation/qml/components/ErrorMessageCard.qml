import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"

Pane {
    id: errorMessageCard

    property variant appStyle: null
    property string errorMessage: ""

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
        margins: 16
    }
    visible: errorMessage.length > 0

    background: Rectangle {
        color: appStyle ? appStyle.error : "#f44336"
        radius: 8
        border.color: appStyle ? appStyle.error : "#f44336"
        border.width: 1
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16

        Label {
            text: errorMessage
            color: "white"
            wrapMode: Text.WordWrap
        }

        Item {
            Layout.fillWidth: true
        }

        ToolButton {
            onClicked: errorMessageCard.errorMessage = ""

            contentItem: ColoredSvg {
                source: "qrc:/icons/icons/close.svg"
                width: 16
                height: 16
                sourceSize.width: 16
                sourceSize.height: 16
                svgColor: "white"
                smooth: true
                mipmap: true
            }
        }
    }
} 