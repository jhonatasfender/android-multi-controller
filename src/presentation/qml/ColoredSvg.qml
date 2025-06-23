import QtQuick 2.15
import QtQuick.Effects

Image {
    id: coloredSvg

    property color svgColor: "white"
    property bool enabled: true

    smooth: true
    mipmap: true
    fillMode: Image.PreserveAspectFit

    layer.enabled: true
    layer.effect: MultiEffect {
        brightness: 1.0
        colorization: 1.0
        colorizationColor: coloredSvg.svgColor
    }
} 