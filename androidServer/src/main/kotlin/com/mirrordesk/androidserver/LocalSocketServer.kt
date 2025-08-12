package com.mirrordesk.androidserver

import android.net.LocalServerSocket
import android.net.LocalSocket
import android.os.Build
import java.io.FileDescriptor
import java.io.IOException

class LocalSocketServer(private val socketName: String) : AutoCloseable {
    private val serverSocket = LocalServerSocket(socketName)

    fun accept(timeoutMs: Int? = null, sendDummyByte: Boolean = false): Pair<LocalSocket, FileDescriptor> {
        val client = serverSocket.accept()
        if (timeoutMs != null && Build.VERSION.SDK_INT >= 21) {
            try {
                client.soTimeout = timeoutMs
            } catch (_: Exception) {
            }
        }
        if (sendDummyByte) {
            try {
                client.outputStream.write(0)
                client.outputStream.flush()
            } catch (_: IOException) {
            }
        }
        return client to client.fileDescriptor
    }

    override fun close() {
        serverSocket.close()
    }
}


