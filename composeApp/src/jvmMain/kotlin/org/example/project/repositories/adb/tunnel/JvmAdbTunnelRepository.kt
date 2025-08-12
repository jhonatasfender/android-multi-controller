package org.example.project.repositories.adb.tunnel

import java.io.File
import java.util.concurrent.TimeUnit

class JvmAdbTunnelRepository : AdbTunnelRepository {
    override fun reverse(
        serial: String,
        deviceSocketName: String,
        localPort: Int,
    ): Boolean {
        val adb = resolveAdbPath()
        val cmd = listOf(adb, "-s", serial, "reverse", "localabstract:$deviceSocketName", "tcp:$localPort")
        return run(cmd)
    }

    override fun forward(
        serial: String,
        localPort: Int,
        deviceSocketName: String,
    ): Boolean {
        val adb = resolveAdbPath()
        val cmd = listOf(adb, "-s", serial, "forward", "tcp:$localPort", "localabstract:$deviceSocketName")
        return run(cmd)
    }

    override fun removeReverse(
        serial: String,
        deviceSocketName: String,
    ): Boolean {
        val adb = resolveAdbPath()
        val cmd = listOf(adb, "-s", serial, "reverse", "--remove", "localabstract:$deviceSocketName")
        return run(cmd)
    }

    override fun removeForward(
        serial: String,
        localPort: Int,
    ): Boolean {
        val adb = resolveAdbPath()
        val cmd = listOf(adb, "-s", serial, "forward", "--remove", "tcp:$localPort")
        return run(cmd)
    }

    private fun run(cmd: List<String>): Boolean {
        return try {
            val pb = ProcessBuilder(cmd)
            pb.directory(File(System.getProperty("user.dir")))
            pb.redirectErrorStream(true)
            val p = pb.start()
            if (!p.waitFor(5, TimeUnit.SECONDS)) {
                p.destroyForcibly()
                false
            } else {
                p.exitValue() == 0
            }
        } catch (_: Exception) {
            false
        }
    }

    private fun resolveAdbPath(): String {
        val fromEnv = System.getenv("ADB") ?: System.getenv("ADB_PATH")
        if (!fromEnv.isNullOrBlank()) return fromEnv
        val fromProp = System.getProperty("adb.path")
        if (!fromProp.isNullOrBlank()) return fromProp
        return "adb"
    }
}
