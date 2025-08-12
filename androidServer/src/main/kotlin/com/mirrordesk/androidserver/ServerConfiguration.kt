package com.mirrordesk.androidserver

data class ServerConfiguration(
    val socketName: String,
    val videoWidth: Int,
    val videoHeight: Int,
    val videoBitRate: Int,
    val maxFpsToEncoder: Float
)

object ServerDefaults {
    const val defaultSocketName: String = "mirrordesk_demo"
    const val defaultVideoWidth: Int = 1280
    const val defaultVideoHeight: Int = 720
    const val defaultVideoBitRate: Int = 6_000_000
    const val defaultMaxFpsToEncoder: Float = 60f
}


