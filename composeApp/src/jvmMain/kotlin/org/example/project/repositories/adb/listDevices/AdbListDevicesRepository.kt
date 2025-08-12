package org.example.project.repositories.adb.listDevices

interface AdbListDevicesRepository {
    suspend fun listDevices(): List<AdbDevice>
}
