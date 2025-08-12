package com.mirrordesk.androidserver

import android.system.Os
import java.io.FileDescriptor
import java.nio.ByteBuffer
import java.nio.ByteOrder

interface VideoPacketWriter {
    fun writeVideoHeader(
        fileDescriptor: FileDescriptor,
        codecId: Int,
        width: Int,
        height: Int,
    )

    fun writeFrame(
        fileDescriptor: FileDescriptor,
        payload: ByteBuffer,
        ptsUs: Long,
        isConfig: Boolean,
        isKeyFrame: Boolean,
    )
}

class DefaultVideoPacketWriter : VideoPacketWriter {
    override fun writeVideoHeader(
        fileDescriptor: FileDescriptor,
        codecId: Int,
        width: Int,
        height: Int,
    ) {
        val header = ByteBuffer.allocate(12).order(ByteOrder.BIG_ENDIAN)
        header.putInt(codecId)
        header.putInt(width)
        header.putInt(height)
        header.flip()
        Os.write(fileDescriptor, header)
    }

    override fun writeFrame(
        fileDescriptor: FileDescriptor,
        payload: ByteBuffer,
        ptsUs: Long,
        isConfig: Boolean,
        isKeyFrame: Boolean,
    ) {
        val configFlag = 1L shl 63
        val keyFlag = 1L shl 62

        var ptsAndFlags = if (isConfig) configFlag else ptsUs
        if (!isConfig && isKeyFrame) ptsAndFlags = ptsAndFlags or keyFlag

        val meta = ByteBuffer.allocate(12).order(ByteOrder.BIG_ENDIAN)
        meta.putLong(ptsAndFlags)
        meta.putInt(payload.remaining())
        meta.flip()
        Os.write(fileDescriptor, meta)

        val temp = ByteArray(64 * 1024)
        var remaining = payload.remaining()
        while (remaining > 0) {
            val n = minOf(temp.size, remaining)
            payload.get(temp, 0, n)
            Os.write(fileDescriptor, temp, 0, n)
            remaining -= n
        }
    }
}
