package org.example.project.repositories.adb.server

interface AndroidServerRepository {
    fun pushAndStartServer(
        serial: String,
        jarPathOnHost: String,
        deviceJarPath: String,
        mainClass: String,
        socketName: String,
        video: Boolean = true,
        control: Boolean = true,
    ): Boolean
}
