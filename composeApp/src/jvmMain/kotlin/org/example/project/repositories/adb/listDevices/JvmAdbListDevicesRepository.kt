package org.example.project.repositories.adb.listDevices

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.util.concurrent.TimeUnit

class JvmAdbListDevicesRepository : AdbListDevicesRepository {
    override suspend fun listDevices(): List<AdbDevice> =
        withContext(Dispatchers.IO) {
            val adbPath = resolveAdbPath()
            val command = listOf(adbPath, "devices", "-l")

            try {
                val pb = ProcessBuilder(command)
                pb.directory(File(System.getProperty("user.dir")))
                pb.redirectErrorStream(true)
                val process = pb.start()

                if (!process.waitFor(5, TimeUnit.SECONDS)) {
                    process.destroyForcibly()
                    return@withContext emptyList()
                }

                val exitCode = process.exitValue()
                val output = process.inputStream.bufferedReader().use { it.readText() }
                if (exitCode != 0) return@withContext emptyList()

                return@withContext parseAdbDevicesOutput(output)
            } catch (_: Exception) {
                return@withContext emptyList()
            }
        }

    private fun resolveAdbPath(): String {
        val fromEnv = System.getenv("ADB") ?: System.getenv("ADB_PATH")
        if (!fromEnv.isNullOrBlank()) return fromEnv
        val fromProp = System.getProperty("adb.path")
        if (!fromProp.isNullOrBlank()) return fromProp
        return "adb"
    }

    internal fun parseAdbDevicesOutput(output: String): List<AdbDevice> {
        val lines =
            output.lines()
                .map { it.trim() }
                .filter { it.isNotEmpty() }

        val devices = mutableListOf<AdbDevice>()
        var inTable = false

        for (line in lines) {
            if (!inTable) {
                if (line.startsWith("List of devices attached")) {
                    inTable = true
                }
                continue
            }
            val parts = line.split(Regex("\\s+"))
            if (parts.isEmpty()) continue
            val serial = parts.getOrNull(0) ?: continue
            val state = parts.getOrNull(1) ?: continue

            val kvs = mutableMapOf<String, String>()
            if (parts.size > 2) {
                for (i in 2 until parts.size) {
                    val token = parts[i]
                    val idx = token.indexOf(':')
                    if (idx > 0 && idx < token.lastIndex) {
                        val k = token.substring(0, idx)
                        val v = token.substring(idx + 1)
                        kvs[k] = v
                    }
                }
            }

            devices +=
                AdbDevice(
                    id = serial,
                    state = state,
                    model = kvs["model"],
                    device = kvs["device"],
                    product = kvs["product"],
                )
        }

        return devices
    }
}
