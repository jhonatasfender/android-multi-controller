package com.mirrordesk.androidserver

import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.media.MediaFormat
import android.os.Build
import android.os.Bundle
import android.view.Surface
import java.nio.ByteBuffer

class MediaCodecVideoEncoder(
    private val codecMimeType: String,
    private val iFrameIntervalSeconds: Int,
    private val repeatPreviousFrameUs: Long,
    private val packetWriter: VideoPacketWriter,
) {
    data class EncoderSession(
        val codec: MediaCodec,
        val inputSurface: Surface,
    )

    fun createSession(
        bitRate: Int,
        width: Int,
        height: Int,
        maxFpsToEncoder: Float,
    ): EncoderSession {
        val codec = MediaCodec.createEncoderByType(codecMimeType)

        val format = MediaFormat()
        format.setString(MediaFormat.KEY_MIME, codecMimeType)
        format.setInteger(MediaFormat.KEY_BIT_RATE, bitRate)
        format.setInteger(MediaFormat.KEY_FRAME_RATE, maxFpsToEncoder.toInt())
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface)
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, iFrameIntervalSeconds)
        format.setLong(MediaFormat.KEY_REPEAT_PREVIOUS_FRAME_AFTER, repeatPreviousFrameUs)
        if (Build.VERSION.SDK_INT >= 29) {
            format.setFloat("max-fps-to-encoder", maxFpsToEncoder)
        }

        codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE)
        val inputSurface = codec.createInputSurface()
        codec.start()
        return EncoderSession(codec, inputSurface)
    }

    fun drainOnce(
        codec: MediaCodec,
        fd: java.io.FileDescriptor,
        bufferInfo: MediaCodec.BufferInfo,
    ): Int {
        val outIndex = codec.dequeueOutputBuffer(bufferInfo, 200_000)
        if (outIndex >= 0) {
            if (bufferInfo.size > 0) {
                val buf: ByteBuffer? = codec.getOutputBuffer(outIndex)
                if (buf != null) {
                    buf.position(bufferInfo.offset)
                    buf.limit(bufferInfo.offset + bufferInfo.size)
                    val isConfig = bufferInfo.flags and MediaCodec.BUFFER_FLAG_CODEC_CONFIG != 0
                    val isKey = bufferInfo.flags and MediaCodec.BUFFER_FLAG_KEY_FRAME != 0
                    packetWriter.writeFrame(fd, buf.slice(), bufferInfo.presentationTimeUs, isConfig, isKey)
                }
            }
            codec.releaseOutputBuffer(outIndex, false)
            return if (bufferInfo.size > 0) bufferInfo.size + 12 else 0
        }
        return -1
    }

    fun requestKeyFrame(codec: MediaCodec) {
        if (Build.VERSION.SDK_INT >= 19) {
            val params = Bundle()
            params.putInt(MediaCodec.PARAMETER_KEY_REQUEST_SYNC_FRAME, 0)
            codec.setParameters(params)
        }
    }

    fun updateBitrate(
        codec: MediaCodec,
        bitRate: Int,
    ) {
        if (Build.VERSION.SDK_INT >= 19) {
            val params = Bundle()
            params.putInt(MediaCodec.PARAMETER_KEY_VIDEO_BITRATE, bitRate)
            codec.setParameters(params)
        }
    }
}
