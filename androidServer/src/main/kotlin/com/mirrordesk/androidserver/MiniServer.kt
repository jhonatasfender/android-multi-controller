package com.mirrordesk.androidserver

import android.media.MediaCodec

object MiniServer {
    private const val MIME = "video/avc"
    private const val CODEC_ID_H264 = 0x68323634
    private const val IFRAME_INTERVAL_SECONDS = 10
    private const val REPEAT_PREV_FRAME_US = 100_000L

    @JvmStatic
    fun main(args: Array<String>) {
        val config = CommandLineArgumentsParser().parse(args)
        LocalSocketServer(config.socketName).use { socketServer ->
            val (client, fd) = socketServer.accept(timeoutMs = 5_000, sendDummyByte = true)

            val packetWriter: VideoPacketWriter = DefaultVideoPacketWriter()
            packetWriter.writeVideoHeader(fd, CODEC_ID_H264, config.videoWidth, config.videoHeight)

            val encoder =
                MediaCodecVideoEncoder(
                    codecMimeType = MIME,
                    iFrameIntervalSeconds = IFRAME_INTERVAL_SECONDS,
                    repeatPreviousFrameUs = REPEAT_PREV_FRAME_US,
                    packetWriter = packetWriter,
                )

            val session =
                encoder.createSession(
                    bitRate = config.videoBitRate,
                    width = config.videoWidth,
                    height = config.videoHeight,
                    maxFpsToEncoder = config.maxFpsToEncoder,
                )

            val bufferInfo = MediaCodec.BufferInfo()
            var lastPtsUs = -1L
            var consecutiveEmptyPolls = 0
            var currentBitrate = config.videoBitRate
            val minBitrate = (config.videoBitRate * 0.25).toInt().coerceAtLeast(200_000)

            while (true) {
                val bytesWritten = encoder.drainOnce(session.codec, fd, bufferInfo)
                if (bytesWritten > 0) {
                    consecutiveEmptyPolls = 0
                    lastPtsUs = bufferInfo.presentationTimeUs
                } else if (bytesWritten == -1) {
                    consecutiveEmptyPolls += 1
                }

                val stalled = consecutiveEmptyPolls >= 10
                if (stalled) {
                    encoder.requestKeyFrame(session.codec)
                    if (currentBitrate > minBitrate) {
                        currentBitrate = (currentBitrate * 0.8).toInt().coerceAtLeast(minBitrate)
                        encoder.updateBitrate(session.codec, currentBitrate)
                    }
                    consecutiveEmptyPolls = 0
                }
            }
        }
    }
}
