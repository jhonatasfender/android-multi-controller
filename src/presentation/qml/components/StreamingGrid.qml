import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"

Rectangle {
    id: root
    
    property alias devices: gridRepeater.model
    property int gridColumns: 2
    property int gridRows: 2
    property bool isStreaming: false
    
    signal deviceSelected(string deviceId)
    signal streamingToggled(string deviceId)
    
    color: Style.background
    
    GridLayout {
        id: mainGrid
        anchors.fill: parent
        anchors.margins: 20
        
        columns: root.gridColumns
        rows: root.gridRows
        
        Repeater {
            id: gridRepeater
            
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 300
                Layout.preferredHeight: 200
                
                color: Style.surface
                border.color: connected ? Style.success : Style.outline
                border.width: connected ? 2 : 1
                radius: 8
                
                readonly property var deviceData: modelData
                readonly property string deviceId: deviceData ? deviceData.id : ""
                readonly property string deviceName: deviceData ? deviceData.name : ""
                readonly property bool connected: deviceData ? deviceData.connected : false
                readonly property bool streaming: deviceData ? deviceData.streaming : false
                
                Column {
                    anchors.centerIn: parent
                    spacing: 15
                    
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 10
                        
                        Rectangle {
                            width: 24
                            height: 24
                            color: parent.parent.parent.connected ? Style.success : Style.error
                            radius: 12
                            
                            Text {
                                anchors.centerIn: parent
                                text: "📱"
                                font.pixelSize: 16
                                color: Style.onSurface
                            }
                        }
                        
                        Text {
                            text: parent.parent.parent.deviceName || parent.parent.parent.deviceId
                            font: Style.body
                            color: Style.onSurface
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    
                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: {
                            if (!parent.parent.connected) return Style.error
                            if (parent.parent.streaming) return Style.success
                            return Style.accent
                        }
                    }
                    
                    Text {
                        text: {
                            if (!parent.parent.connected) return "Disconnected"
                            if (parent.parent.streaming) return "Streaming"
                            return "Connected"
                        }
                        font: Style.caption
                        color: Style.onSurfaceVariant
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    
                    Rectangle {
                        width: 220
                        height: 140
                        anchors.horizontalCenter: parent.horizontalCenter
                        
                        color: Style.background
                        border.color: parent.parent.streaming ? Style.success : Style.outline
                        border.width: parent.parent.streaming ? 2 : 1
                        radius: 4
                        
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2
                            
                            Rectangle {
                                anchors.fill: parent
                                color: Style.surfaceVariant
                                visible: !parent.parent.parent.streaming
                                
                                Column {
                                    anchors.centerIn: parent
                                    spacing: 8
                                    
                                    Rectangle {
                                        width: 32
                                        height: 32
                                        color: Style.onSurfaceVariant
                                        radius: 16
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: "📱"
                                            font.pixelSize: 24
                                            color: Style.surface
                                        }
                                    }
                                    
                                    Text {
                                        text: parent.parent.parent.parent.connected ? "Click to start streaming" : "Device not connected"
                                        font: Style.caption
                                        color: Style.onSurfaceVariant
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                }
                            }
                            
                            Rectangle {
                                anchors.fill: parent
                                color: Style.background
                                visible: parent.parent.parent.streaming
                                
                                Column {
                                    anchors.centerIn: parent
                                    spacing: 4
                                    
                                    Text {
                                        text: "📱 Live Stream"
                                        font: Style.caption
                                        color: Style.success
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                    
                                    Text {
                                        text: parent.parent.parent.parent.deviceName
                                        font: Style.body
                                        color: Style.onSurface
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                    
                                    Text {
                                        text: "H.264 • 30 FPS"
                                        font: Style.caption
                                        color: Style.onSurfaceVariant
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                }
                                
                                Rectangle {
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.margins: 4
                                    width: 50
                                    height: 16
                                    color: Style.success
                                    radius: 2
                                    
                                    Text {
                                        anchors.centerIn: parent
                                        text: "LIVE"
                                        font.pixelSize: 8
                                        color: Style.onSurface
                                        font.bold: true
                                    }
                                }
                                
                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.bottom: parent.bottom
                                    anchors.margins: 4
                                    width: 60
                                    height: 16
                                    color: Qt.rgba(0, 0, 0, 0.5)
                                    radius: 2
                                    
                                    Text {
                                        anchors.centerIn: parent
                                        text: "8080"
                                        font.pixelSize: 8
                                        color: Style.onSurface
                                    }
                                }
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (parent.parent.parent.connected) {
                                    root.streamingToggled(parent.parent.parent.deviceId)
                                }
                            }
                        }
                    }
                    
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8
                        
                        Button {
                            text: parent.parent.streaming ? "Stop" : "Start"
                            font.pixelSize: 10
                            width: 60
                            height: 24
                            enabled: parent.parent.connected
                            
                            background: Rectangle {
                                color: parent.parent.streaming ? Style.error : Style.success
                                opacity: parent.enabled ? 1.0 : 0.3
                                radius: 4
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                color: Style.onSurface
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            
                            onClicked: {
                                root.streamingToggled(parent.parent.deviceId)
                            }
                        }
                        
                        Button {
                            text: "Full"
                            font.pixelSize: 10
                            width: 50
                            height: 24
                            enabled: parent.parent.streaming
                            
                            background: Rectangle {
                                color: Style.primary
                                opacity: parent.enabled ? 1.0 : 0.3
                                radius: 4
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                color: Style.onSurface
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            
                            onClicked: {
                                root.deviceSelected(parent.parent.deviceId)
                            }
                        }
                        
                        Button {
                            text: "ℹ"
                            font.pixelSize: 12
                            width: 24
                            height: 24
                            enabled: parent.parent.connected
                            
                            background: Rectangle {
                                color: Style.surface
                                border.color: Style.outline
                                border.width: 1
                                radius: 4
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                color: Style.onSurface
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            
                            onClicked: {
                                console.log("Device info for:", parent.parent.deviceId)
                            }
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
        color: Style.surface
        border.color: Style.outline
        border.width: 1
        radius: 8
        visible: !gridRepeater.model || gridRepeater.model.length === 0
        
        Column {
            anchors.centerIn: parent
            spacing: 15
            
            Rectangle {
                width: 48
                height: 48
                color: Style.onSurfaceVariant
                radius: 24
                anchors.horizontalCenter: parent.horizontalCenter
                
                Text {
                    anchors.centerIn: parent
                    text: "📱"
                    font.pixelSize: 36
                    color: Style.surface
                }
            }
            
            Text {
                text: "No devices connected"
                font: Style.body
                color: Style.onSurfaceVariant
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: "Connect Android devices via ADB\nand refresh to see them here"
                font: Style.caption
                color: Style.onSurfaceVariant
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}