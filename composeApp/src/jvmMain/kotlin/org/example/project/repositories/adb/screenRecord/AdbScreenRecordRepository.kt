package org.example.project.repositories.adb.screenRecord

interface AdbScreenRecordRepository {
    fun startScreencapPngLoopExecOut(serial: String): Process

    fun startScreenRecordExecOut(
        serial: String,
        width: Int? = null,
        height: Int? = null,
        bitrateMbps: Int? = null,
        maxFps: Int? = null,
    ): Process
}
