import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import DeviceController 1.0
import "../"

Item {
    id: multiDeviceMirrorView

    property var deviceList: multiDeviceMirrorViewModel ? multiDeviceMirrorViewModel.devices : []
    property variant appStyle: null
    property bool isMirroring: multiDeviceMirrorViewModel ? multiDeviceMirrorViewModel.isMirroring : false
    property int gridColumns: multiDeviceMirrorViewModel ? multiDeviceMirrorViewModel.gridColumns : 2
    property int gridRows: multiDeviceMirrorViewModel ? multiDeviceMirrorViewModel.gridRows : 2

    signal deviceClicked(var device)
    signal startMirroringClicked()
    signal stopMirroringClicked()

    Component {
        id: deviceMirrorCardComponent
        
        Rectangle {
            id: deviceMirrorCard

            property var device: null
            property variant appStyle: null
            property bool isMirroring: false
            property var screenImage: screenImage

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
                        property int imageVersion: 0
                        source: device && device.id ? "image://deviceimages/" + device.id + "?v=" + imageVersion : ""
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        mipmap: true
                        visible: isMirroring && status === Image.Ready
                        cache: false
                        
                        onStatusChanged: {
                            if (status === Image.Ready) {
                            }
                        }
                        
                        onSourceChanged: {
                        }
                        
                        onVisibleChanged: {
                        }
                        
                        Component.onCompleted: {
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 4
                        color: "transparent"
                        border.color: screenImage.status === Image.Ready ? "green" : "red"
                        border.width: 3
                        visible: isMirroring
                        
                        Text {
                            anchors.centerIn: parent
                            text: screenImage.status === Image.Ready ? "IMAGE READY" : "IMAGE LOADING..."
                            color: screenImage.status === Image.Ready ? "green" : "red"
                            font.pixelSize: 14
                            font.bold: true
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 4
                        color: appStyle ? appStyle.background : "#121212"
                        radius: 2
                        border.color: appStyle ? appStyle.divider : "#2d2d2d"
                        border.width: 1
                        visible: !isMirroring

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

            Connections {
                target: multiDeviceMirrorViewModel
                function onScreenCaptured(deviceId) {
                    for (let i = 0; i < deviceRepeater.count; i++) {
                        const loader = deviceRepeater.itemAt(i)
                        
                        if (loader && loader.item && loader.item.device && loader.item.device.id === deviceId) {
                            const screenImage = loader.item.screenImage
                            if (screenImage) {
                                screenImage.imageVersion++
                                
                                // Clear the source first
                                screenImage.source = ""
                                
                                // Create a timer to set the new source with updated version
                                const timer = Qt.createQmlObject('import QtQuick 2.15; Timer { interval: 10; repeat: false; }', loader)
                                timer.triggered.connect(function() {
                                    const newSource = "image://deviceimages/" + deviceId + "?v=" + screenImage.imageVersion
                                    screenImage.source = newSource
                                    timer.destroy()
                                })
                                timer.start()
                            }
                            break
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: appStyle ? appStyle.background : "#121212"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        Pane {
            Layout.fillWidth: true
            Layout.preferredHeight: 80

            background: Rectangle {
                color: appStyle ? appStyle.surface : "#1e1e1e"
                radius: 8
                border.color: appStyle ? appStyle.divider : "#2d2d2d"
                border.width: 1
            }

            contentItem: RowLayout {
                spacing: 16

                Rectangle {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48
                    radius: 24
                    color: appStyle ? appStyle.accent : "#2196f3"
                    opacity: appStyle ? appStyle.accentOpacity : 0.1

                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/icons/icons/phone.svg"
                        width: 24
                        height: 24
                        sourceSize.width: 24
                        sourceSize.height: 24
                        smooth: true
                        mipmap: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Label {
                        text: qsTr("Multi-Device Mirroring")
                        font.pixelSize: 18
                        font.weight: Font.Medium
                        color: appStyle ? appStyle.foreground : "#ffffff"
                    }

                    Label {
                        text: qsTr("Real-time screen mirroring for multiple devices")
                        font.pixelSize: 14
                        color: appStyle ? appStyle.secondaryText : "#b3ffffff"
                    }
                }

                RowLayout {
                    spacing: 12

                    Button {
                        text: isMirroring ? qsTr("Stop Mirroring") : qsTr("Start Mirroring")
                        background: Rectangle {
                            color: isMirroring ? (appStyle ? appStyle.buttonDanger : "#f44336") : (appStyle ? appStyle.buttonPrimary : "#2196f3")
                            radius: 4
                        }
                        onClicked: {
                            if (multiDeviceMirrorViewModel) {
                                if (isMirroring) {
                                    multiDeviceMirrorViewModel.stopMirroring()
                                } else {
                                    multiDeviceMirrorViewModel.startMirroring()
                                }
                            }
                        }
                    }

                    Button {
                        text: qsTr("Grid Settings")
                        background: Rectangle {
                            color: appStyle ? appStyle.buttonSecondary : "#666666"
                            radius: 4
                        }
                        onClicked: gridSettingsDialog.open()
                    }
                }
            }
        }

        Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true

            background: Rectangle {
                color: appStyle ? appStyle.surface : "#1e1e1e"
                radius: 8
                border.color: appStyle ? appStyle.divider : "#2d2d2d"
                border.width: 1
            }

            contentItem: Item {
                anchors.fill: parent

                GridLayout {
                    id: deviceGrid
                    anchors.fill: parent
                    anchors.margins: 16
                    columns: gridColumns
                    rowSpacing: 8
                    columnSpacing: 8

                    Repeater {
                        id: deviceRepeater
                        model: deviceList

                        Loader {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            sourceComponent: deviceMirrorCardComponent
                            onLoaded: {
                                item.device = modelData
                                item.appStyle = multiDeviceMirrorView.appStyle
                                item.isMirroring = multiDeviceMirrorView.isMirroring
                                item.clicked.connect(function() { deviceClicked(modelData) })
                            }
                            
                            Connections {
                                target: multiDeviceMirrorView
                                function onIsMirroringChanged() {
                                    if (item) {
                                        item.isMirroring = multiDeviceMirrorView.isMirroring
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 300
                    height: 200
                    color: appStyle ? appStyle.background : "#121212"
                    radius: 8
                    border.color: appStyle ? appStyle.divider : "#2d2d2d"
                    border.width: 1
                    visible: deviceList.length === 0

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 16

                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            source: "qrc:/icons/icons/phone.svg"
                            width: 64
                            height: 64
                            sourceSize.width: 64
                            sourceSize.height: 64
                            smooth: true
                            mipmap: true
                        }

                        Label {
                            text: qsTr("No Devices Connected")
                            font.pixelSize: 18
                            font.weight: Font.Medium
                            color: appStyle ? appStyle.foreground : "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Label {
                            text: qsTr("Connect devices to start mirroring")
                            font.pixelSize: 14
                            color: appStyle ? appStyle.secondaryText : "#b3ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: gridSettingsDialog
        title: qsTr("Grid Settings")
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 300

        contentItem: ColumnLayout {
            spacing: 16

            Label {
                text: qsTr("Configure grid layout for device mirroring")
                font.pixelSize: 14
                color: appStyle ? appStyle.foreground : "#ffffff"
                wrapMode: Text.WordWrap
            }

            RowLayout {
                spacing: 16

                Label {
                    text: qsTr("Columns:")
                    font.pixelSize: 14
                    color: appStyle ? appStyle.foreground : "#ffffff"
                }

                SpinBox {
                    id: columnsSpinBox
                    from: 1
                    to: 4
                    value: gridColumns
                    onValueChanged: {
                        if (multiDeviceMirrorViewModel) {
                            multiDeviceMirrorViewModel.setGridColumns(value)
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: qsTr("Rows:")
                    font.pixelSize: 14
                    color: appStyle ? appStyle.foreground : "#ffffff"
                }

                SpinBox {
                    id: rowsSpinBox
                    from: 1
                    to: 4
                    value: gridRows
                    onValueChanged: {
                        if (multiDeviceMirrorViewModel) {
                            multiDeviceMirrorViewModel.setGridRows(value)
                        }
                    }
                }
            }

            Label {
                text: qsTr("Grid Preview:")
                font.pixelSize: 14
                font.weight: Font.Medium
                color: appStyle ? appStyle.foreground : "#ffffff"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                color: appStyle ? appStyle.background : "#121212"
                radius: 4
                border.color: appStyle ? appStyle.divider : "#2d2d2d"
                border.width: 1

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    columns: columnsSpinBox.value
                    rowSpacing: 4
                    columnSpacing: 4

                    Repeater {
                        model: columnsSpinBox.value * rowsSpinBox.value

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: appStyle ? appStyle.accent : "#2196f3"
                            opacity: 0.3
                            radius: 2
                        }
                    }
                }
            }
        }

        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            onAccepted: gridSettingsDialog.accept()
            onRejected: gridSettingsDialog.reject()
        }
    }
} 