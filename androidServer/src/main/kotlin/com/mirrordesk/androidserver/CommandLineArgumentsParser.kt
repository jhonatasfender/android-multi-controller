package com.mirrordesk.androidserver

class CommandLineArgumentsParser {
    fun parse(args: Array<String>): ServerConfiguration {
        var socketName = ServerDefaults.DEFAULT_SOCKET_NAME
        var width = ServerDefaults.DEFAULT_VIDEO_WIDTH
        var height = ServerDefaults.DEFAULT_VIDEO_HEIGHT
        var bitRate = ServerDefaults.DEFAULT_VIDEO_BITRATE
        var maxFpsToEncoder = ServerDefaults.DEFAULT_MAX_FPS_TO_ENCODER

        var i = 0
        while (i < args.size) {
            when (args[i]) {
                "--socket" ->
                    args.getOrNull(i + 1)?.let {
                        socketName = it
                        i++
                    }
                "--w" ->
                    args.getOrNull(i + 1)?.toIntOrNull()?.let {
                        width = it
                        i++
                    }
                "--h" ->
                    args.getOrNull(i + 1)?.toIntOrNull()?.let {
                        height = it
                        i++
                    }
                "--bitrate" ->
                    args.getOrNull(i + 1)?.toIntOrNull()?.let {
                        bitRate = it
                        i++
                    }
                "--maxfps" ->
                    args.getOrNull(i + 1)?.toFloatOrNull()?.let {
                        maxFpsToEncoder = it
                        i++
                    }
            }
            i++
        }

        return ServerConfiguration(
            socketName = socketName,
            videoWidth = width,
            videoHeight = height,
            videoBitRate = bitRate,
            maxFpsToEncoder = maxFpsToEncoder,
        )
    }
}
