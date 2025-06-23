import QtQuick 2.15
import QtQuick.Controls 2.15
import "../"

Rectangle {
    id: loadingOverlay

    property variant appStyle: null

    property bool isLoading: false

    anchors.fill: parent
    color: appStyle ? appStyle.background : "#121212"
    opacity: isLoading ? 0.8 : 0
    visible: isLoading

    Behavior on opacity {
        NumberAnimation {
            duration: 200
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: isLoading
    }
} 