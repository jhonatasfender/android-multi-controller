package org.example.project.core.stream

import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.toComposeImageBitmap
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancelAndJoin
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import org.jetbrains.skia.Image
import java.io.ByteArrayOutputStream
import java.io.InputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder

class DeviceVideoStreamClient(
    private val screenRecordRepo: org.example.project.repositories.adb.screenRecord.AdbScreenRecordRepository,
    private val serial: String,
) {
    data class State(
        val running: Boolean = false,
        val bytesReceived: Long = 0L,
        val error: String? = null,
        val lastFrame: ImageBitmap? = null,
    )

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    private val _state = MutableStateFlow(State())
    val state: StateFlow<State> = _state

    @Volatile private var process: Process? = null
    private var readerJob: Job? = null

    fun start() {
        if (readerJob != null) return
        readerJob =
            scope.launch {
                _state.value = _state.value.copy(running = true, error = null)
                try {
                    val p = screenRecordRepo.startScreencapPngLoopExecOut(serial)
                    process = p
                    p.inputStream.use { input ->
                        readPngStream(input)
                    }
                } catch (t: Throwable) {
                    _state.value = _state.value.copy(error = (t.message ?: t::class.simpleName))
                } finally {
                    _state.value = _state.value.copy(running = false)
                }
            }
    }

    fun stop() {
        val job = readerJob
        readerJob = null
        process?.destroyForcibly()
        process = null
        if (job != null) {
            scope.launch {
                try {
                    job.cancelAndJoin()
                } catch (_: Throwable) {
                }
            }
        }
    }

    private fun readPngStream(input: InputStream) {
        val signature = byteArrayOf(0x89.toByte(), 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A)
        while (true) {
            if (!resyncToSignature(input, signature)) break
            val baos = ByteArrayOutputStream(512 * 1024)
            baos.write(signature)
            while (true) {
                val header = readFully(input, 8) ?: return
                baos.write(header)
                val length = readIntBigEndian(header, 0)
                val type = String(header, 4, 4)
                val data = readFully(input, length) ?: return
                baos.write(data)
                val crc = readFully(input, 4) ?: return
                baos.write(crc)
                if (type == "IEND") {
                    val bytes = baos.toByteArray()
                    val image = Image.makeFromEncoded(bytes).toComposeImageBitmap()
                    val s = _state.value
                    _state.value = s.copy(bytesReceived = s.bytesReceived + bytes.size, lastFrame = image)
                    break
                }
            }
        }
    }

    private fun readIntBigEndian(
        bytes: ByteArray,
        offset: Int,
    ): Int {
        return ByteBuffer.wrap(bytes, offset, 4).order(ByteOrder.BIG_ENDIAN).int
    }

    private fun readFully(
        input: InputStream,
        length: Int,
    ): ByteArray? {
        val out = ByteArray(length)
        var off = 0
        while (off < length) {
            val r = input.read(out, off, length - off)
            if (r <= 0) return null
            off += r
        }
        return out
    }

    private fun resyncToSignature(
        input: InputStream,
        signature: ByteArray,
    ): Boolean {
        val window = ByteArray(signature.size)
        var filled = 0
        while (filled < window.size) {
            val r = input.read(window, filled, window.size - filled)
            if (r <= 0) return false
            filled += r
        }
        if (window.contentEquals(signature)) return true
        while (true) {
            if (window.contentEquals(signature)) return true
            val b = input.read()
            if (b == -1) return false
            for (i in 0 until window.size - 1) {
                window[i] = window[i + 1]
            }
            window[window.size - 1] = b.toByte()
        }
    }
}
