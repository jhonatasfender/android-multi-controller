package com.mirrordesk.androidserver

data class ServerConfiguration(
    val socketName: String,
    val videoWidth: Int,
    val videoHeight: Int,
    val videoBitRate: Int,
    val maxFpsToEncoder: Float,
)

object ServerDefaults {
    const val DEFAULT_SOCKET_NAME: String = "mirrordesk_demo"
    const val DEFAULT_VIDEO_WIDTH: Int = 1280
    const val DEFAULT_VIDEO_HEIGHT: Int = 720
    const val DEFAULT_VIDEO_BITRATE: Int = 6_000_000
    const val DEFAULT_MAX_FPS_TO_ENCODER: Float = 60f
}
