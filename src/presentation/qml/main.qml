import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import DeviceController 1.0
import "components"

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1200
    height: 800
    title: qsTr("Multi-Device Android Controller")

    property bool isFullscreen: false

    property int originalVisibility: Window.Windowed
    property int originalWidth: 1200
    property int originalHeight: 800
    property int originalX: 0
    property int originalY: 0

    Material.theme: Material.Dark
    Material.accent: Material.Blue
    Material.primary: Material.BlueGrey

    minimumWidth: 800
    minimumHeight: 600

    Style {
        id: appStyle
    }

    LoggingBridge {
        id: logBridge
    }

    function toggleFullscreen() {
        if (isFullscreen) {
            visibility = originalVisibility
            width = originalWidth
            height = originalHeight
            x = originalX
            y = originalY
            isFullscreen = false
            logBridge.info("MainWindow", "Exited fullscreen mode")
        } else {
            originalVisibility = visibility
            originalWidth = width
            originalHeight = height
            originalX = x
            originalY = y
            visibility = Window.FullScreen
            isFullscreen = true
            logBridge.info("MainWindow", "Entered fullscreen mode")
        }
    }

    Shortcut {
        sequence: "F11"
        onActivated: toggleFullscreen()
    }

    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (isFullscreen) {
                toggleFullscreen()
            }
        }
    }

    Component.onCompleted: {
        logBridge.info("MainWindow", "Application window initialized")
        logBridge.info("MainWindow", "Theme: Dark, Accent: Blue, Primary: BlueGrey")
        logBridge.info("MainWindow", "Fullscreen shortcuts: F11 to toggle, Escape to exit")
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            appStyle: mainWindow.appStyle
            isLoading: deviceListViewModel.isLoading
            connectedDeviceCount: deviceListViewModel.connectedDeviceCount
            deviceList: deviceListViewModel.devices

            onRefreshClicked: deviceListViewModel.refreshDevices()
            onConnectAllClicked: deviceListViewModel.connectToAllDevices()
            onDisconnectAllClicked: deviceListViewModel.disconnectFromAllDevices()
            onConnectDeviceClicked: function (deviceId) {
                deviceListViewModel.connectToDevice(deviceId)
            }
            onDisconnectDeviceClicked: function (deviceId) {
                deviceListViewModel.disconnectFromDevice(deviceId)
            }
            onControlDeviceClicked: function (device) {
                mainContentArea.navigateToDeviceControl(device)
            }
            onMultiDeviceMirrorClicked: function () {
                mainContentArea.navigateToMultiDeviceMirror()
            }
        }

        MainContentArea {
            id: mainContentArea
            appStyle: mainWindow.appStyle
            isLoading: deviceListViewModel.isLoading
            errorMessage: deviceListViewModel.errorMessage
        }
    }

    ToolButton {
        id: fullscreenButton
        anchors {
            top: parent.top
            right: parent.right
            margins: 16
        }
        z: 1000

        background: Rectangle {
            color: parent.pressed ? appStyle.buttonPrimaryPressed :
                parent.hovered ? appStyle.buttonPrimaryHovered :
                    appStyle.buttonPrimary
            radius: 4
            opacity: 0.8
        }

        contentItem: ColoredSvg {
            source: mainWindow.isFullscreen ? "qrc:/icons/icons/exit-fullscreen.svg" : "qrc:/icons/icons/fullscreen.svg"
            width: 20
            height: 20
            sourceSize.width: 20
            sourceSize.height: 20
            svgColor: "white"
            smooth: true
            mipmap: true
        }

        onClicked: toggleFullscreen()

        ToolTip.visible: hovered
        ToolTip.text: mainWindow.isFullscreen ? qsTr("Exit fullscreen (F11)") : qsTr("Enter fullscreen (F11)")
    }

    Rectangle {
        id: fullscreenOverlay
        anchors.fill: parent
        color: "transparent"
        z: 999

        visible: false

        Rectangle {
            id: statusIndicator
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
                topMargin: 50
            }
            width: statusText.width + 40
            height: statusText.height + 20
            radius: 8
            color: appStyle.surface
            opacity: 0.9

            Label {
                id: statusText
                anchors.centerIn: parent
                text: mainWindow.isFullscreen ? qsTr("Fullscreen Mode") : qsTr("Windowed Mode")
                color: appStyle.foreground
                font.pixelSize: 14
                font.weight: Font.Medium
            }
        }

        SequentialAnimation {
            id: showAnimation
            running: false

            PropertyAction {
                target: fullscreenOverlay
                property: "visible"
                value: true
            }

            PauseAnimation {
                duration: 1500
            }

            PropertyAction {
                target: fullscreenOverlay
                property: "visible"
                value: false
            }
        }
    }

    onIsFullscreenChanged: {
        showAnimation.start()
    }
}