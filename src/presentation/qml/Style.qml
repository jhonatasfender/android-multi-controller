import QtQuick 2.15

QtObject {
    readonly property color primary: "#1976d2"
    readonly property color accent: "#2196f3"
    readonly property color surface: "#1e1e1e"
    readonly property color background: "#121212"
    readonly property color foreground: "#ffffff"
    readonly property color error: "#f44336"
    readonly property color success: "#4caf50"
    readonly property color onSurface: "#ffffff"
    readonly property color onSurfaceVariant: "#b3ffffff"
    readonly property color outline: "#424242"
    readonly property color surfaceVariant: "#2d2d2d"
    readonly property color onBackground: "#ffffff"
    readonly property color secondary: "#90caf9"
    readonly property color onSecondary: "#000000"
    
    // Propriedades para fonts
    readonly property var colors: this
    readonly property var fonts: this
    readonly property font title: Qt.font({ pixelSize: 20 })
    readonly property font body: Qt.font({ pixelSize: 14 })
    readonly property font caption: Qt.font({ pixelSize: 12 })
}   