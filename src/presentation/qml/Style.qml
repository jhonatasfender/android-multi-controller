import QtQuick 2.15
import QtQuick.Controls.Material 2.15

QtObject {
    id: style

    property var logBridge: LoggingBridge

    readonly property color primary: Material.primary || "#1976d2"
    readonly property color accent: Material.accent || "#2196f3"
    readonly property color surface: "#1e1e1e"
    readonly property color background: "#121212"
    readonly property color foreground: "#ffffff"
    readonly property color secondaryText: "#b3ffffff"
    readonly property color divider: "#2d2d2d"
    readonly property color hintText: "#80ffffff"
    readonly property color error: "#f44336"
    readonly property color success: "#4caf50"

    readonly property color statusOnline: "#4caf50"
    readonly property color statusOffline: "#f44336"
    readonly property color statusBusy: "#ff9800"
    readonly property color statusError: "#f44336"

    readonly property color buttonPrimary: "#2196f3"
    readonly property color buttonPrimaryPressed: "#1976d2"
    readonly property color buttonPrimaryHovered: "#42a5f5"
    readonly property color buttonSecondary: "#666666"
    readonly property color buttonSecondaryPressed: "#555555"
    readonly property color buttonSecondaryHovered: "#777777"
    readonly property color buttonDanger: "#f44336"
    readonly property color buttonDangerPressed: "#d32f2f"
    readonly property color buttonDangerHovered: "#ef5350"
    readonly property color buttonSuccess: "#4caf50"
    readonly property color buttonSuccessPressed: "#388e3c"
    readonly property color buttonSuccessHovered: "#66bb6a"

    readonly property color textPrimary: "#ffffff"
    readonly property color textSecondary: "#b3ffffff"
    readonly property color textHint: "#80ffffff"
    readonly property color textDisabled: "#666666"

    readonly property color surfaceLight: "#2d2d2d"
    readonly property color surfaceMedium: "#1e1e1e"
    readonly property color surfaceDark: "#121212"

    readonly property real accentOpacity: 0.1
    readonly property real hoverOpacity: 0.05
    readonly property real pressedOpacity: 0.1
    readonly property real disabledOpacity: 0.38
}