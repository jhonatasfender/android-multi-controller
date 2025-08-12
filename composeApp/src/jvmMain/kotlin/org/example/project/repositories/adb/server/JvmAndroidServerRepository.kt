package org.example.project.repositories.adb.server

import java.io.File
import java.util.concurrent.TimeUnit

class JvmAndroidServerRepository : AndroidServerRepository {
    override fun pushAndStartServer(
        serial: String,
        jarPathOnHost: String,
        deviceJarPath: String,
        mainClass: String,
        socketName: String,
        video: Boolean,
        control: Boolean
    ): Boolean {
        val adb = resolveAdbPath()
        val hostJar = File(jarPathOnHost)
        if (!hostJar.exists()) return false

        if (!run(listOf(adb, "-s", serial, "push", hostJar.absolutePath, deviceJarPath))) return false

        val args = buildList {
            add(adb); add("-s"); add(serial); add("shell")
            add("CLASSPATH=${deviceJarPath}")
            add("app_process"); add("/"); add(mainClass)
            add("--socket"); add(socketName)
            if (video) { add("--video") }
            if (control) { add("--control") }
        }
        return run(args)
    }

    private fun run(cmd: List<String>): Boolean {
        return try {
            val pb = ProcessBuilder(cmd)
            pb.directory(File(System.getProperty("user.dir")))
            pb.redirectErrorStream(true)
            val p = pb.start()
            if (!p.waitFor(8, TimeUnit.SECONDS)) {
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


