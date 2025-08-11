import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import DeviceController 1.0
import "../"

Item {
    id: multiDeviceMirrorView

    property var multiDeviceMirrorViewModel: null
    property var deviceList: multiDeviceMirrorViewModel ? multiDeviceMirrorViewModel.devices : []
    property variant appStyle: null
    property bool isMirroring: multiDeviceMirrorViewModel ? multiDeviceMirrorViewModel.isMirroring : false
    property int gridColumns: multiDeviceMirrorViewModel ? multiDeviceMirrorViewModel.gridColumns : 2
    property int gridRows: multiDeviceMirrorViewModel ? multiDeviceMirrorViewModel.gridRows : 2

    signal deviceClicked(var device)
    signal startMirroringClicked()
    signal stopMirroringClicked()
    
    Column {
        anchors.fill: parent
        spacing: 20
        
        Rectangle {
            width: parent.width
            height: 60
            color: Style.colors.surface
            border.color: Style.colors.outline
            border.width: 1
            radius: 8
            
            Row {
                anchors.centerIn: parent
                spacing: 20
                
                Text {
                    text: "Multi-Device Streaming"
                    font: Style.fonts.title
                    color: Style.colors.onSurface
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Button {
                    text: multiDeviceMirrorView.isMirroring ? "Stop Streaming" : "Start Streaming"
                    Material.background: multiDeviceMirrorView.isMirroring ? Style.colors.error : Style.colors.primary
                    Material.foreground: "#FFFFFF"
                    anchors.verticalCenter: parent.verticalCenter
                    
                    onClicked: {
                        if (multiDeviceMirrorView.isMirroring) {
                            multiDeviceMirrorView.stopMirroringClicked()
                        } else {
                            multiDeviceMirrorView.startMirroringClicked()
                        }
                    }
                }
                
                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 10
                    
                    Text {
                        text: "Grid:"
                        font: Style.fonts.body
                        color: Style.colors.onSurface
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    SpinBox {
                        id: gridColumnsSpinBox
                        from: 1
                        to: 4
                        value: multiDeviceMirrorView.gridColumns
                        anchors.verticalCenter: parent.verticalCenter
                        
                        onValueModified: {
                            if (multiDeviceMirrorViewModel) {
                                multiDeviceMirrorViewModel.setGridColumns(value)
                            }
                        }
                    }
                    
                    Text {
                        text: "×"
                        font: Style.fonts.body
                        color: Style.colors.onSurface
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    SpinBox {
                        id: gridRowsSpinBox
                        from: 1
                        to: 4
                        value: multiDeviceMirrorView.gridRows
                        anchors.verticalCenter: parent.verticalCenter
                        
                        onValueModified: {
                            if (multiDeviceMirrorViewModel) {
                                multiDeviceMirrorViewModel.setGridRows(value)
                            }
                        }
                    }
                }
            }
        }
        
        StreamingGrid {
            width: parent.width
            height: parent.height - 80
            
            devices: multiDeviceMirrorView.deviceList
            gridColumns: multiDeviceMirrorView.gridColumns
            gridRows: multiDeviceMirrorView.gridRows
            isStreaming: multiDeviceMirrorView.isMirroring
            
            onDeviceSelected: function(deviceId) {
                console.log("Device selected for fullscreen:", deviceId)
                
                // Future: Implement fullscreen view for selected device
                var device = multiDeviceMirrorViewModel.getDeviceById(deviceId)
                if (device) {
                    console.log("Opening fullscreen for device:", device.name, "- ID:", device.id)
                    // TODO: Navigate to fullscreen view
                } else {
                    console.warn("Device not found:", deviceId)
                }
            }
            
            onStreamingToggled: function(deviceId) {
                console.log("Streaming toggled for device:", deviceId)
                if (multiDeviceMirrorViewModel) {
                    var success = multiDeviceMirrorViewModel.toggleStreamingForDevice(deviceId)
                    if (success) {
                        console.log("Successfully toggled streaming for device:", deviceId)
                    } else {
                        console.warn("Failed to toggle streaming for device:", deviceId)
                    }
                } else {
                    console.error("MultiDeviceMirrorViewModel not available")
                }
            }
        }
    }
    
    Connections {
        target: multiDeviceMirrorViewModel
        function onDevicesChanged() {
            console.log("Devices changed")
        }
        function onIsMirroringChanged() {
            console.log("Is mirroring changed:", multiDeviceMirrorViewModel.isMirroring)
        }
        function onMirroringStarted() {
            console.log("Mirroring started")
        }
        function onMirroringStopped() {
            console.log("Mirroring stopped")
        }
    }
} 