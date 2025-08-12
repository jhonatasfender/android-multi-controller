package org.example.project.repositories.adb.screenRecord

import java.io.File

class JvmAdbScreenRecordRepository : AdbScreenRecordRepository {
    override fun startScreencapPngLoopExecOut(serial: String): Process {
        val adbPath = resolveAdbPath()
        val cmd =
            listOf(
                adbPath,
                "-s",
                serial,
                "exec-out",
                "sh",
                "-c",
                "while true; do screencap -p; done",
            )
        val pb = ProcessBuilder(cmd)
        pb.directory(File(System.getProperty("user.dir")))
        return pb.start()
    }

    override fun startScreenRecordExecOut(
        serial: String,
        width: Int?,
        height: Int?,
        bitrateMbps: Int?,
        maxFps: Int?,
    ): Process {
        val adbPath = resolveAdbPath()
        val cmd =
            mutableListOf(
                adbPath,
                "-s",
                serial,
                "exec-out",
                "screenrecord",
                "--output-format=h264",
                "-",
            )
        if (width != null && height != null) {
            cmd += listOf("--size", "${width}x$height")
        }
        if (bitrateMbps != null) {
            val bps = (bitrateMbps.toLong() * 1_000_000L).toString()
            cmd += listOf("--bit-rate", bps)
        }
        if (maxFps != null) {
            cmd += listOf("--bugreport")
        }
        cmd += listOf("--time-limit", "3600")
        val pb = ProcessBuilder(cmd)
        pb.directory(File(System.getProperty("user.dir")))
        return pb.start()
    }

    private fun resolveAdbPath(): String {
        val fromEnv = System.getenv("ADB") ?: System.getenv("ADB_PATH")
        if (!fromEnv.isNullOrBlank()) return fromEnv
        val fromProp = System.getProperty("adb.path")
        if (!fromProp.isNullOrBlank()) return fromProp
        return "adb"
    }
}
