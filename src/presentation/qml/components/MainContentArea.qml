import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"

Item {
    id: mainContentArea

    property variant appStyle: null
    property bool isLoading: false
    property string errorMessage: ""

    Layout.fillWidth: true
    Layout.fillHeight: true

    Rectangle {
        anchors.fill: parent
        color: appStyle ? appStyle.background : "#121212"
    }

    LoadingOverlay {
        appStyle: mainContentArea.appStyle
        isLoading: mainContentArea.isLoading
    }

    ErrorMessageCard {
        appStyle: mainContentArea.appStyle
        errorMessage: mainContentArea.errorMessage
    }

    StackView {
        id: mainStack
        anchors.fill: parent
        anchors.topMargin: mainContentArea.errorMessage.length > 0 ? 80 : 16
        anchors.margins: 16

        initialItem: welcomeViewComponent
    }

    Component {
        id: welcomeViewComponent

        Loader {
            source: "qrc:/qml/WelcomeView.qml"
            property var appStyle: mainContentArea.appStyle
        }
    }

    Component {
        id: deviceControlViewComponent

        Loader {
            source: "qrc:/qml/DeviceControlView.qml"
            property var appStyle: mainContentArea.appStyle
        }
    }

    Component {
        id: multiDeviceMirrorViewComponent

        Loader {
            source: "qrc:/qml/components/MultiDeviceMirrorView.qml"
            property var appStyle: mainContentArea.appStyle
        }
    }

    function navigateToDeviceControl(device) {
        var deviceControlView = deviceControlViewComponent.createObject(mainStack, {
            "device": device,
            "appStyle": mainContentArea.appStyle
        })
        mainStack.push(deviceControlView)
    }

    function navigateToMultiDeviceMirror() {
        var multiDeviceMirrorView = multiDeviceMirrorViewComponent.createObject(mainStack, {
            "appStyle": mainContentArea.appStyle
        })
        mainStack.push(multiDeviceMirrorView)
    }
} 