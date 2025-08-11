import QtQuick 2.15

Image {
    id: coloredSvg

    property color svgColor: "white"
    property alias color: coloredSvg.svgColor
    property bool enabled: true

    smooth: true
    mipmap: true
    fillMode: Image.PreserveAspectFit
    
    opacity: enabled ? 1.0 : 0.5
    
} 